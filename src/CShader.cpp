//*****************************************************************
//* CShader.cpp
//* -----------
//*
//* Este fichero incluye las definiciones de los m�todos de
//* la clase CShader y la implementaci�n de funciones auxiliares.
//*
//* Algunos de los m�todos que aqu� se presentan deber�n ser
//* completados por el alumno de forma obligatoria. Otros podr�n
//* modificarse para dar  soporte a partes opcionales.
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
//*****************************************************************

/*******************************************************************

Modificado para la pr�ctica de Rendering Avanzado por Sergio Galindo
y Luis Hijarrubia. 2014

Las mayores modificaciones son para pasar el c�digo a openGL 3.3
Y se pueda elegir si se van a pasar "in"s de v�rtices, normales y coordenadas

*********************************************************************/

#include "CShader.h"
#include <iostream>
#include <string.h>

//Funciones auxiliar.

//Esta funci�n permite inicializar el shader
//geometrico y se ejecuta antes de hacer el
//link del shader. Como par�metro recibe el
//identificador del programa.
CShader::CShader(bool conNormales,bool conTextura,
                 const char *vShaderFile,const char *fShaderFile,
                 const char *gShaderFile,void (*gShaderInit)(unsigned int))
{
  initializeOpenGLFunctions( );

  vshader = loadShader(vShaderFile,GL_VERTEX_SHADER);
  fshader = loadShader(fShaderFile,GL_FRAGMENT_SHADER);
  if (gShaderFile!=NULL)
    gshader = loadShader(gShaderFile,GL_GEOMETRY_SHADER);

  program= glCreateProgram();
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  if (gShaderFile!=NULL)
    glAttachShader(program, gshader);
  if (gShaderInit !=NULL)
    gShaderInit(program);

  glBindAttribLocation(program, 0, "inVertex");
  inVertex=0;
  if (conNormales){
    glBindAttribLocation(program, 1, "inNormal");
    inNormal=1;
  }
  if (conTextura){
    glBindAttribLocation(program, 2, "inTexCoord");
    inTexCoord=2;
  }

  glLinkProgram(program);

  int linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked){
    GLint logLen;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
    char *logString= new char[logLen];
    glGetProgramInfoLog(program, logLen, NULL,logString);
    std::cout << "Error: " << logString <<std::endl;
    delete logString;
    glDeleteProgram(program);
    program=0;
    exit(-1);
  }

//inVertex = glGetAttribLocation(program,"inVertex");
//if (conNormales)
//inNormal = glGetAttribLocation(program,"inNormal");
//if (conTextura)
//inTexCoord = glGetAttribLocation(program,"inTexCoord");

}

CShader::~CShader(void){
  glDetachShader(program, vshader);
  glDetachShader(program, fshader);
  glDetachShader(program, gshader);
  glDeleteShader(vshader);
  glDeleteShader(fshader);
  glDeleteShader(gshader);
  glDeleteProgram(program);
}



void CShader::activate(){
  glUseProgram(program);
}

void CShader::deactivate(){
  glUseProgram(0);
}

unsigned int CShader::getID(){
  return program;
}

unsigned int CShader::getInVertex(){return inVertex;}
unsigned int CShader::getInNormal(){return inNormal;}
unsigned int CShader::getInTexCoord(){return inTexCoord;}

unsigned int CShader::compileShader(const char* source, GLenum type){
  GLuint shader;
  GLint fileLen=GLint(strlen(source));

  shader = glCreateShader(type);
  glShaderSource (shader, 1,(const GLchar **) &source, (const GLint *)&fileLen);

  glCompileShader(shader);

  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if(!compiled){
    GLint logLen;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    char *logString= new char[logLen];
    glGetShaderInfoLog(shader, logLen, NULL,logString);
    std::cout << "Error: " << logString <<std::endl;
    delete logString;
    glDeleteShader(shader);
    exit(-1);
  }
  return shader;
}


unsigned int CShader::loadShader (const char *fileName, GLenum type)
{
//  std::cout << std::endl;
//  std::cout << "________________________________________________________________" << std::endl;
//  std::cout << "Cargando Shader: " << fileName << std::endl;
//  std::cout << "________________________________________________________________" << std::endl;

  char *source;
  source = readShaderFile(fileName);
  if (source == NULL){
    std::cerr << "Error leyedo el fichero: " << fileName << std::endl;
    exit(-1);
  }
  GLuint shader = compileShader(source, type);
  delete source;
  return shader;
}



//*****************************************************************
//* Funciones auxiliares (Lectura de ficheros)
//* ------------------------------------------
//*
//* Las funciones, variables y metdos descritos a continuacion
//* NO REQUERIR�N MODIFICACI�N ALGUNA.
//*
//*****************************************************************
#include <fstream>

char *CShader::readShaderFile(const char *fileName)
{
  //Se carga el fichero
  std::ifstream file;
  file.open(fileName, std::ios::in);
  if(!file) return 0;

  //Se calcula la longitud del fichero
  file.seekg(0,std::ios::end);
  unsigned int fileLen = file.tellg();
  file.seekg(std::ios::beg);

  //Se lee el fichero
  char *source = new char[fileLen+1];

  int i=0;
  while(file.good())
  {
    source[i]=file.get();
    if (!file.eof()) i++;
    else fileLen=i;
  }
  source[fileLen]= '\0';
  file.close();

  return source;
}
