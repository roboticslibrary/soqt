/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
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

//  $Id$

#ifndef SOQT_POPUPMENU_H
#define SOQT_POPUPMENU_H

#include <Inventor/Qt/SoQtBasic.h>

#include <Inventor/Qt/widgets/SoAnyPopupMenu.h>

// *************************************************************************

struct MenuRecord;
struct ItemRecord;

class SOQT_DLL_EXPORT SoQtPopupMenu : public QObject, public SoAnyPopupMenu
{
  typedef SoAnyPopupMenu inherited;
  Q_OBJECT

public:
  SoQtPopupMenu(void);
  virtual ~SoQtPopupMenu(void);

  virtual int NewMenu( char * name, int menuid = -1 );
  virtual int GetMenu( char * name );
  virtual void SetMenuTitle( int id, char * title );
  virtual char * GetMenuTitle( int id );

  virtual int NewMenuItem( char * name, int itemid = -1 );
  virtual int GetMenuItem( char * name );
  virtual void SetMenuItemTitle( int itemid, char * title );
  virtual char * GetMenuItemTitle( int itemid );
  virtual void SetMenuItemEnabled( int itemid, SbBool enabled );
  virtual SbBool GetMenuItemEnabled( int itemid );
  virtual SbBool GetMenuItemMarked( int itemid );

  virtual void AddMenu( int menuid, int submenuid, int pos = -1 );
  virtual void AddMenuItem( int menuid, int itemid, int pos = -1 );
  virtual void AddSeparator( int menuid, int pos = -1 );
  virtual void RemoveMenu( int menuid );
  virtual void RemoveMenuItem( int itemid );

  virtual void PopUp( QWidget * inside, int x, int y );

protected:
  virtual void _setMenuItemMarked( int itemid, SbBool marked );

  MenuRecord * getMenuRecord( int menuid );
  ItemRecord * getItemRecord( int itemid );
  MenuRecord * createMenuRecord( char * name );
  ItemRecord * createItemRecord( char * name );

private slots:
  void itemActivation( int itemid );

private:
  SbPList * menus;
  SbPList * items;

};  // class SoQtPopupMenu

// *************************************************************************

#endif // ! SOQT_POPUPMENU_H
