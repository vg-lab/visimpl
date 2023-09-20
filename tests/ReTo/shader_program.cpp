/*
 *  Copyright (c) 2015-2022 VG-Lab/URJC.
 *
 *  Authors: Gael Rial Costas  <gael.rial.costas@urjc.es>
 *
 *  This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 *  This library is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License version 3.0 as published
 *  by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define BOOST_TEST_MODULE reto_shader_program

#include <reto/reto.h>
#include <boost/test/unit_test.hpp>
#include <visimpl_test_utils.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>

using namespace reto;

BOOST_AUTO_TEST_CASE(test_reto_program_shader)
{
    test_utils::initOpenGLContext();

    const std::string vsShader = (
            "#version 430 core\n"
            "layout(location = 0) in vec3 position;\n"
            "uniform mat4 MVP;\n"
            "void main() {\n"
            " gl_Position = MVP * vec4(position, 1.0);\n"
            "}"
    );

    const std::string fsShader = (
            "#version 430 core\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            " fragColor = vec4(1.0);\n"
            "}"
    );

    const std::string gsShader = (
            "#version 430 core\n"
            "layout (points) in;\n"
            "layout (line_strip) out;\n"
            "layout (max_vertices = 8) out;\n"
            "void main() {\n"
            "    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);\n"
            "    EmitVertex();\n"
            "    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);\n"
            "    EmitVertex();\n"
            "    EndPrimitive();\n"
            "}"
    );

    //reto::Camera c;
    reto::ShaderProgram prog;
    bool vsCreate = prog.loadVertexShaderFromText(vsShader);
    BOOST_CHECK(vsCreate == true);

    bool fsCreate = prog.loadFragmentShaderFromText(fsShader);
    BOOST_CHECK(fsCreate == true);

    bool compile = prog.compileAndLink();
    BOOST_CHECK(compile == true);

    prog.addUniform("MVP");
    BOOST_CHECK(prog["MVP"] == 0);
    BOOST_CHECK(prog.uniform("MVP") == 0);


    reto::ShaderProgram prog2;
    vsCreate = prog2.loadVertexShaderFromText(vsShader);
    BOOST_CHECK(vsCreate == true);

    fsCreate = prog2.loadFragmentShaderFromText(fsShader);
    BOOST_CHECK(fsCreate == true);

    bool gsCreate = prog2.loadGeometryShaderFromText(gsShader);
    BOOST_CHECK(gsCreate == true);

    prog2.create();
    compile = prog2.link();

    BOOST_CHECK(compile == true);
    prog2.use();

    BOOST_CHECK(prog2.getGeometryInputType() == GL_POINTS);
    BOOST_CHECK(prog2.getGeometryOutputType() == GL_LINE_STRIP);
    BOOST_CHECK(prog2.getGeometryMaxOutput() == 8);

    test_utils::terminateOpenGLContext();
}
