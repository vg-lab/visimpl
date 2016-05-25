//*****************************************************************
//* CShader.cpp
//* -----------
//*
//* Este fichero incluye las declaraciones de la clase CShader
//*
//* Podr�n a�adirse nuevos m�todos y/o atributos para dar soporte
//* a partes opcionales. NO SE REQUIERE SU MODIFICACI�N EN LAS
//* PARTES OBLIGATORIAS, NI EN LA MAYOR�A DE LAS OPCIONALES.
//*
//* --------------------------------------------------------------
//* Descargo de responsabilidad.
//*
//* El c�digos se proporciona �nicamente a modo de ilustraci�n.
//* El ejemplo no se ha verificado a fondo bajo todas las
//* condiciones. No se puede garantizar ni dar por supuesta la
//* fiabilidad, la posibilidad de servicio, ni del funcionamiento
//* programa.
//*
//* Este fichero se distribuye bajo una licencia no exclusiva
//* de derechos de autor para pudiendo utilizar el c�digo para
//* generar funciones similares que se ajusten a sus necesidades.
//*
//* El c�digo que aqu� se incluyen se ofrecen "TAL CUAL" sin
//* garant�as de ning�n tipo.
//*
//* Copyright (C) 2012 by Marcos Garc�a Lorenzo (GMRV - URJC)
//* http://www.gmrv.es/
//******************************************************************

/*******************************************************************

Modificado para la pr�ctica de Rendering Avanzado por Sergio Galindo
y Luis Hijarrubia. 2014

*********************************************************************/

#pragma once

// #include <GL/glew.h>
#include <QOpenGLFunctions>

class CShader : public QOpenGLFunctions

{
public:
  CShader(bool conNormales,bool conTextura,
          const char *vShaderFile,
          const char *fShaderFile,
          const char *gShaderFile = 0L,
//Esta funci�n permite inicializar el shader
//geometrico y se ejecuta antes de hacer el
//link del shader. Como par�metro recibe el
//identificador del programa.
          void (*gShaderInit)(unsigned int) = 0L
    );

  ~CShader(void);

  void activate();
  void deactivate();

  unsigned int getID();

  unsigned int getInVertex();
  unsigned int getInNormal();
  unsigned int getInTexCoord();

  char *readShaderFile(const char *fileName);
  unsigned int compileShader(const char* source, GLenum type);
  unsigned int loadShader (const char *fileName, GLenum type);


protected:

  unsigned int vshader;
  unsigned int fshader;
  unsigned int gshader;
  unsigned int program;

  unsigned int inVertex;
  unsigned int inNormal;
  unsigned int inTexCoord;

};
