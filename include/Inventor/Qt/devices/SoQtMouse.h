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

#ifndef __SOQTMOUSE_H__
#define __SOQTMOUSE_H__

#include <Inventor/Qt/devices/SoQtDevice.h>

class SoMouseButtonEvent;
class SoLocation2Event;


class SoQtMouse : public SoQtDevice
{
  typedef SoQtDevice inherited;
  
public:
  // FIXME: remove "SoQtMouse" in name, as its redundant. 19990620 mortene.
  enum SoQtMouseEventMask {
    ButtonPressMask = 0x01,
    ButtonReleaseMask = 0x02,
    PointerMotionMask = 0x04,
    ButtonMotionMask = 0x08,

    SO_QT_ALL_MOUSE_EVENTS = 0x0f,
  };

  SoQtMouse(SoQtMouseEventMask mask = SO_QT_ALL_MOUSE_EVENTS);
  virtual ~SoQtMouse();

  virtual void enable(QWidget * w, SoQtEventHandler f, void * data);
  virtual void disable(QWidget * w, SoQtEventHandler f, void * data);

  virtual const SoEvent * translateEvent(QEvent * event);

private:
  SoMouseButtonEvent * buttonevent;
  SoLocation2Event * locationevent;
  SoQtMouseEventMask eventmask;
};

#endif // !__SOQTMOUSE_H__
