if ( OPENGL_FOUND )
  message(FATAL_ERROR
    "FindnvidiaOpengL should be run before finding package OpenGL")
endif( )

file(GLOB NVIDIA_OPENGL_gl_LIBRARY_PATHS_HINT
  /usr/lib/nvidia-* )

find_library(NVIDIA_OPENGL_gl_LIBRARY
  GL
  PATHS
  ${NVIDIA_OPENGL_gl_LIBRARY_PATHS_HINT}
  NO_DEFAULT_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
)

if ( NVIDIA_OPENGL_gl_LIBRARY )
  set(OPENGL_gl_LIBRARY ${NVIDIA_OPENGL_gl_LIBRARY})
  set(NVIDIAOPENGL_FOUND 1)
endif( )
