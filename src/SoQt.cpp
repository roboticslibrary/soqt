/**************************************************************************\
 * 
 *  Copyright (C) 1998-1999 by Systems in Motion.  All rights reserved.
 *
 *  This file is part of the Coin library.
 *
 *  This file may be distributed under the terms of the Q Public License
 *  as defined by Troll Tech AS of Norway and appearing in the file
 *  LICENSE.QPL included in the packaging of this file.
 *
 *  If you want to use Coin in applications not covered by licenses
 *  compatible with the QPL, you can contact SIM to aquire a
 *  Professional Edition license for Coin.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class SoQt SoQt.h Inventor/Qt/SoQt.h
  \brief The SoQt class takes care of Qt initialization and event dispatching.

  This is the "application-wide" class with solely static methods
  handling initialization and event processing tasks. You must use this
  class in any application built on top of the SoQt library. Typical usage
  is as follows (complete application code):

  \code
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>

#include <Inventor/nodes/SoCube.h>

#include <qwidget.h>

int
main(int argc, char **argv)
{
  // Initialize SoQt and Open Inventor libraries. This returns a main
  // window to use.
  QWidget * mainwin = SoQt::init(argv[0]);

   // Make a dead simple scene graph, only containing a single cube.
  SoCube * cube = new SoCube;

  // Use one of the convenient viewer classes.
  SoQtExaminerViewer * eviewer = new SoQtExaminerViewer(mainwin);
  eviewer->setSceneGraph(cube);
  eviewer->show();

  // Pop up the main window.
  SoQt::show(mainwin);
  // Loop until exit.
  SoQt::mainLoop();
  return 0;
}
  \endcode
   
  For general information on the Qt GUI toolkit, see the web site for
  Troll Tech (makers of Qt): <http://www.troll.no>.

  \sa SoQtComponent
 */

#include <Inventor/Qt/SoQt.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/SbTime.h>

#if SOQT_DEBUG
#include <Inventor/errors/SoDebugError.h>
#endif // SOQT_DEBUG

#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qtimer.h>


// *************************************************************************

QWidget * SoQt::mainWidget = NULL;
QApplication * SoQt::appobject = NULL;
QTimer * SoQt::idletimer = NULL;
QTimer * SoQt::timerqueuetimer = NULL;
QTimer * SoQt::delaytimeouttimer = NULL;
SoQt * SoQt::slotobj = NULL;

// *************************************************************************

/*!
  This method is provided for easier porting/compatibility with the
  Open Inventor SoXt component classes. It just adds dummy \a argc and
  \a argv arguments and calls the SoQt::init() method below.
*/
QWidget * 
SoQt::init(const char * const appName, const char * const className)
{
  if (appName) {
    char buf[1025];
    strncpy(buf, appName, 1024);
    char *array[1] = { buf };
    // fake argc, argv for QApplication
    return SoQt::init(1, array, appName, className);
  }
  else {
    return SoQt::init(0, NULL, appName, className);
  }
}

/*!
  Calls \a SoDB::init(), \a SoNodeKit::init(), \a SoInteraction::init().
  Assumes you are creating your own QApplication and main widget.
  \a topLevelWidget should be your application's main widget.
*/
void 
SoQt::init(QWidget * const topLevelWidget)
{
#if SOQT_DEBUG
  if (SoQt::mainWidget) {
    SoDebugError::postWarning("SoQt::init",
			      "This method should be called only once.");
    return;
  }
#endif // SOQT_DEBUG

  SoDB::init();
  SoNodeKit::init();
  SoInteraction::init();

  SoDB::getSensorManager()->setChangedCallback(SoQt::sensorQueueChanged, NULL);
  SoQt::mainWidget = topLevelWidget;
}

/*!
  Initializes the SoQt component toolkit library, as well as the Open Inventor
  library.

  Calls \a SoDB::init(), \a SoNodeKit::init(), \a SoInteraction::init(), and
  creates a QApplication and constructs and returns a  main widget for
  you
  
  \sa getApplication()
 */
QWidget *
SoQt::init(int argc, char ** argv,
	   const char * const appName,
	   const char * const className)
{
#if SOQT_DEBUG
  if (SoQt::appobject || SoQt::mainWidget) {
    SoDebugError::postWarning("SoQt::init",
			      "This method should be called only once.");
    return SoQt::mainWidget;
  }
#endif // SOQT_DEBUG

  SoQt::appobject = new QApplication(argc, argv);
  QWidget * mainw = new QMainWindow(NULL, className);
  SoQt::init(mainw);

#if 0 // debug
  SoDebugError::postInfo("SoQt::init", "setCaption('%s')", appName);
#endif // debug

  if (appName) SoQt::mainWidget->setCaption(appName);
  SoQt::appobject->setMainWidget(SoQt::mainWidget);
  return SoQt::mainWidget;
}

/*!
  \internal

  This function gets called whenever something has happened to any of
  the sensor queues. It starts or reschedules a timer which will trigger
  when a sensor is ripe for plucking.
 */
void
SoQt::sensorQueueChanged(void *)
{
  SoSensorManager * sm = SoDB::getSensorManager();

  // Allocate Qt timers on first call.

  if (!SoQt::timerqueuetimer) {
    SoQt::timerqueuetimer = new QTimer;
    QObject::connect(SoQt::timerqueuetimer, SIGNAL(timeout()),
		     SoQt::soqt_instance(), SLOT(slot_timedOutSensor()));
    SoQt::idletimer = new QTimer;
    QObject::connect(SoQt::idletimer, SIGNAL(timeout()),
		     SoQt::soqt_instance(), SLOT(slot_idleSensor()));
    SoQt::delaytimeouttimer = new QTimer;
    QObject::connect(SoQt::delaytimeouttimer, SIGNAL(timeout()),
		     SoQt::soqt_instance(), SLOT(slot_delaytimeoutSensor()));
  }


  // Set up timer queue timeout if necessary.

  SbTime t;
  if (sm->isTimerSensorPending(t)) {
#if 0 // debug
    SoDebugError::postInfo("SoQt::sensorQueueChanged",
			   "timersensor pending");
#endif // debug

    SbTime interval = t - SbTime::getTimeOfDay();
    if (!SoQt::timerqueuetimer->isActive())
      SoQt::timerqueuetimer->start(interval.getMsecValue(), TRUE);
    else
      SoQt::timerqueuetimer->changeInterval(interval.getMsecValue());
  }
  else if (SoQt::timerqueuetimer->isActive()) {
    SoQt::timerqueuetimer->stop();
  }


  // Set up idle notification for delay queue processing if necessary.

  if (sm->isDelaySensorPending()) {
#if 0 // debug
    SoDebugError::postInfo("SoQt::sensorQueueChanged",
			   "delaysensor pending");
#endif // debug

    if (!SoQt::idletimer->isActive()) SoQt::idletimer->start(0, TRUE);

    if (!SoQt::delaytimeouttimer->isActive()) {
      unsigned long timeout = SoDB::getDelaySensorTimeout().getMsecValue();
      SoQt::delaytimeouttimer->start(timeout, TRUE);
    }
  }
  else {
    if (SoQt::idletimer->isActive()) SoQt::idletimer->stop();
    if (SoQt::delaytimeouttimer->isActive()) SoQt::delaytimeouttimer->stop();
  }
}

/*!
  \internal

  This is provided for convenience when debugging the library. Should make
  it easier to find memory leaks.
 */
void
SoQt::clean(void)
{
  delete SoQt::mainWidget; SoQt::mainWidget = NULL;
  delete SoQt::appobject; SoQt::appobject = NULL;

  delete SoQt::timerqueuetimer; SoQt::timerqueuetimer = NULL;
  delete SoQt::idletimer; SoQt::idletimer = NULL;
  delete SoQt::delaytimeouttimer; SoQt::delaytimeouttimer = NULL;

  delete SoQt::slotobj; SoQt::slotobj = NULL;

#if defined(__COIN__)
  SoInteraction::clean();
  SoNodeKit::clean();
  SoDB::getSensorManager()->setChangedCallback(NULL, NULL);
  SoDB::clean();
#endif // __COIN__
}

/*!
  This is the event dispatch loop. It doesn't return until
  \a QApplication::quit() or \a QApplication::exit() is called (which
  is also done automatically by Qt whenever the user closes an application's
  main widget).
 */
void 
SoQt::mainLoop(void)
{
  // We need to process immediate sensors _before_ any events are
  // processed. This is done by installing an eventFilter on the
  // global QApplication object.
  qApp->installEventFilter(SoQt::soqt_instance());

  qApp->exec();

  SoQt::clean();
}

/*!
  Returns a pointer to the Qt QApplication which was instantiated in
  init().
 */
QApplication *
SoQt::getApplication(void)
{
  return SoQt::appobject;
}

/*!
  Returns a pointer to the Qt QWidget which is the main widget for the
  application. When this widget gets closed, SoQt::mainLoop() will
  return (unless the close event is caught by the user).

  \sa getShellWidget()
 */
QWidget *
SoQt::getTopLevelWidget(void)
{
  return SoQt::mainWidget;
}

/*!
  Returns a pointer to the Qt QWidget which is the top level widget for the
  given QWidget \a w. This is just a convenience function provided for
  easier porting of Open Inventor applications based on SoXt components,
  as you can do the same thing by calling the QWidget::topLevelWidget()
  method directly on \a w.

  \sa getTopLevelWidget()
 */
QWidget *
SoQt::getShellWidget(const QWidget * const w)
{
#if SOQT_DEBUG
  if (w == NULL) {
    SoDebugError::postWarning("SoQt::getShellWidget",
			      "Called with NULL pointer.");
    return NULL;
  }
#endif // SOQT_DEBUG

  return w->topLevelWidget();
}

/*!
  This method is provided for easier porting/compatibility with the
  Open Inventor SoXt component classes. It will call QWidget::show() and
  QWidget::raise() on the provided \a widget pointer.

  \sa hide()
*/
void 
SoQt::show(QWidget * const widget)
{
#if SOQT_DEBUG
  if (widget == NULL) {
    SoDebugError::postWarning("SoQt::show",
			      "Called with NULL pointer.");
    return;
  }
#endif // SOQT_DEBUG

#if 0 // debug
  SoDebugError::postInfo("SoQt::show-1",
			 "size %p: (%d, %d)",
			 widget,
			 widget->size().width(), widget->size().height());
#endif // debug
			 
  widget->adjustSize();

#if 0 // debug
  SoDebugError::postInfo("SoQt::show-2",
			 "size %p: (%d, %d)",
			 widget,
			 widget->size().width(), widget->size().height());
#endif // debug
			 
  widget->show();
  widget->raise();

#if 0 // debug
  SoDebugError::postInfo("SoQt::show-3",
			 "size %p: (%d, %d)",
			 widget,
			 widget->size().width(), widget->size().height());
#endif // debug
}

/*!
  This method is provided for easier porting/compatibility with the
  Open Inventor SoXt component classes. It will call QWidget::hide() on the
  provided \a widget pointer.

  \sa show()
 */
void 
SoQt::hide(QWidget * const widget)
{
#if SOQT_DEBUG
  if (widget == NULL) {
    SoDebugError::postWarning("SoQt::hide",
			      "Called with NULL pointer.");
    return;
  }
#endif // SOQT_DEBUG

  widget->hide();
}
  
/*!
  This method is provided for easier porting of applications based on the
  Open Inventor SoXt component classes. It will call QWidget::resize() on the
  provided \a w widget pointer.

  \sa getWidgetSize()
 */
void 
SoQt::setWidgetSize(QWidget * const w, const SbVec2s & size)
{
#if SOQT_DEBUG
  if (w == NULL) {
    SoDebugError::postWarning("SoQt::setWidgetSize",
			      "Called with NULL pointer.");
    return;
  }
  if((size[0] <= 0) || (size[1] <= 0)) {
    SoDebugError::postWarning("SoQt::setWidgetSize",
			      "Called with invalid dimension(s): (%d, %d).",
			      size[0], size[1]);
    return;
  }
#endif // SOQT_DEBUG

#if 0 // debug
  SoDebugError::postInfo("SoQt::setWidgetSize",
			 "resize %p: (%d, %d)",
			 w, size[0], size[1]);
#endif // debug
			 
  w->resize(size[0], size[1]);
}


/*!
  This method is provided for easier porting/compatibility with the
  Open Inventor SoXt component classes. It will do the same as calling
  QWidget::size() (except that we're returning an SbVec2s).

  \sa setWidgetSize()
 */
SbVec2s 
SoQt::getWidgetSize(const QWidget * const w)
{
#if SOQT_DEBUG
  if (w == NULL) {
    SoDebugError::postWarning("SoQt::getWidgetSize",
			      "Called with NULL pointer.");
    return SbVec2s(0, 0);
  }
#endif // SOQT_DEBUG

  return SbVec2s(w->width(), w->height());
}

/*!
  This will pop up an error dialog. It's just a simple wrap-around for
  the Qt QMessageBox::warning() call, provided for easier porting from
  applications using the Open Inventor SoXt component classes.

  If \a widget is \a NULL, the dialog will be modal for the whole
  application (all windows will be blocked for input). If not,
  only the window for the given \a widget will be blocked.

  \a dialogTitle is the title of the dialog box. \a errorStr1 and
  \a errorStr2 contains the text which will be shown in the dialog box.

  There will only be a single "Ok" button for the user to press.
 */
void 
SoQt::createSimpleErrorDialog(QWidget * const widget, 
			      const char * const dialogTitle, 
			      const char * const errorStr1, 
			      const char * const errorStr2)
{
#if SOQT_DEBUG
  if (dialogTitle == NULL) {
    SoDebugError::postWarning("SoQt::createSimpleErrorDialog",
			      "Called with NULL dialogTitle pointer.");
  }
  if (errorStr1 == NULL) {
    SoDebugError::postWarning("SoQt::createSimpleErrorDialog",
			      "Called with NULL error string pointer.");
  }
#endif // SOQT_DEBUG

  SbString title(dialogTitle ? dialogTitle : "");
  SbString errstr(errorStr1 ? errorStr1 : "");

  if (errorStr2) {
    errstr += '\n';
    errstr += errorStr2;
  }

  QMessageBox::warning(widget, title.getString(), errstr.getString());
}  


// *************************************************************************

/*!
  \internal

  We're using the singleton pattern to create a single SoQt object instance
  (a dynamic object is needed for attaching slots to signals -- this is
  really a workaround for some silliness in the Qt design).
 */
SoQt *
SoQt::soqt_instance(void)
{
  if (!SoQt::slotobj) SoQt::slotobj = new SoQt;
  return SoQt::slotobj;
}

/*!
  \internal

  Uses an event filter on qApp to be able to process immediate delay
  sensors before any system events.
 */
bool
SoQt::eventFilter(QObject *, QEvent *)
{
  if (SoDB::getSensorManager()->isDelaySensorPending())
    SoDB::getSensorManager()->processImmediateQueue();

  return FALSE;
}

/*!
  \internal

  A timer sensor is ready for triggering, so tell the sensor manager object
  to process the queue.
*/
void
SoQt::slot_timedOutSensor()
{
#if 0 // debug
  SoDebugError::postInfo("SoQt::timedOutSensor",
			 "processing timer queue");
#endif // debug
  SoDB::getSensorManager()->processTimerQueue();

  // The change callback is _not_ called automatically from
  // SoSensorManager after the process methods, so we need to
  // explicitly trigger it ourselves here.
  SoQt::sensorQueueChanged(NULL);
}

/*!
  \internal

  The system is idle, so we're going to process the queue of delay type
  sensors.
*/
void
SoQt::slot_idleSensor()
{
#if 0 // debug
  SoDebugError::postInfo("SoQt::idleSensor",
			 "processing delay queue");
#endif // debug

  SoDB::getSensorManager()->processDelayQueue(TRUE);

  // The change callback is _not_ called automatically from
  // SoSensorManager after the process methods, so we need to
  // explicitly trigger it ourselves here.
  SoQt::sensorQueueChanged(NULL);
}

/*!
  \internal

  The delay sensor timeout point has been reached, so process the delay
  queue even though the system is not idle.
 */
void
SoQt::slot_delaytimeoutSensor()
{
#if 0 // debug
  SoDebugError::postInfo("SoQt::delaytimeoutSensor",
			 "processing delay queue");
#endif // debug

  SoDB::getSensorManager()->processDelayQueue(FALSE);

  // The change callback is _not_ called automatically from
  // SoSensorManager after the process methods, so we need to
  // explicitly trigger it ourselves here.
  SoQt::sensorQueueChanged(NULL);
}
