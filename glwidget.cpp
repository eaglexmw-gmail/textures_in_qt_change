/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      clearColor(Qt::black),
      program(0)
{
    //memset(textures, 0, sizeof(textures));
    textures = nullptr;
    m_width_ = 250;
}

GLWidget::~GLWidget()
{
    makeCurrent();
    vbo.destroy();
    delete textures;
    delete program;
    doneCurrent();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(250, 250);
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(170.0/255.0, 170.0/255.0, 170.0/255.0,1.0);
    
    makeObject();

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc = "#version 330 core\n"
        "layout (location = 0) in highp vec3 vertex;\n"   //"attribute highp vec4 vertex;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in mediump vec4 texCoord;\n" //"attribute mediump vec4 texCoord;\n"
        "out vec3 ourColor;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 vertex4 = vec4(vertex, 1.0);\n"
        "    gl_Position = matrix * vertex4;\n"
        "    ourColor = aColor;\n"
        "    texc = texCoord;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc = "#version 330 core\n"
        "uniform sampler2D texture1;\n"
        "varying mediump vec4 texc;\n"
        "in vec3 ourColor;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture(texture1, texc.st);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(fshader);
    program->link();

    program->bind();
    program->setUniformValue("texture1", 0);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 m;
    m.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 4.0f, 15.0f);
    m.translate(0.0f, 0.0f, -10.0f);
    program->setUniformValue("matrix", m);
    
    program->enableAttributeArray(0);
    program->enableAttributeArray(1);
    program->enableAttributeArray(2);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat));
    program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
    program->setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(GLfloat), 2, 8 * sizeof(GLfloat));

    textures->bind();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
void GLWidget::resizeGL(int width, int height)
{
	m_width_ = width;
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::makeObject()
{
    QImage tmpImage = QImage(QString(":/images/side%1.png").arg(1)).mirrored();
    unsigned int width_ = tmpImage.width();
    GLfloat scale = (((GLfloat)m_width_)/(2*width_));
    textures = new QOpenGLTexture(tmpImage);

    static const GLfloat coords[] = {
        +scale, -scale, -scale, 1.0f, 0.0f, 0.0f,   1, 1,
        -scale, -scale, -scale, 0.0f, 1.0f, 0.0f,   0, 1,
        -scale, +scale, -scale, 0.0f, 0.0f, 1.0f,   0, 0,
        +scale, +scale, -scale, 1.0f, 1.0f, 0.0f,   1, 0,
    };

    vbo.create();
    vbo.bind();
    vbo.allocate(coords, sizeof(coords));
}
