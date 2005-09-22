/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2005 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#ifndef SOQT_GLMATERIALSPHERE_H
#define SOQT_GLMATERIALSPHERE_H

// FIXME: not in use yet. 19990620 mortene.

# include <qgl.h>
# include <qwidget.h>

class SoMaterial;

class SoGLMaterialSphere : public QGLWidget
{
  Q_OBJECT

public:
  SoGLMaterialSphere(QWidget * parent = NULL, const char * name = NULL);

  void setRepaint(bool whatever);

public slots:
  void setAmbient(float);
  void setDiffuse(float);
  void setSpecular(float);
  void setEmission(float);
  void setShininess(float);
  void setTransparency(float);
  void setMaterial(SoMaterial * mat);
  void repaintSphere();

protected:
  void paintGL();
  void resizeGL(int w, int h);
  void initializeGL();

private:
  void drawSphere();
  void drawBackground();
  void calculateSpherePoints(float * points, int max_points);

  SoMaterial * material;
  float ambientLight[4];
  float diffuseLight[4];
  float specularLight[4];
  bool doRepaint;
};

#endif // ! SOQT_GLMATERIALSPHERE_H
