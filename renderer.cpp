// 2x errors: "malloc"
// 4x erorrs:  glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);
// refreshtimelabel should be replaced by better code now

// #define GLUT_DISABLE_ATEXIT_HACK what is this?

#include "renderer.h"
#include "viewer.h"
#include <math.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

extern stateInfo *state;
extern stateInfo *tempConfig;

//static functions in Knossos 3.x
static GLuint renderWholeSkeleton(Byte callFlag) { return 0;}
static GLuint renderSuperCubeSkeleton(Byte callFlag) { return 0;}
static GLuint renderActiveTreeSkeleton(Byte callFlag) { return 0;}
static uint32_t renderCylinder(Coordinate *base, float baseRadius, Coordinate *top, float topRadius) { return 0;}
static uint32_t renderSphere(Coordinate *pos, float radius) { return 0;}
static uint32_t renderText(Coordinate *pos, char *string) { return 0;}
static uint32_t updateDisplayListsSkeleton() { return 0;}
static uint32_t renderSegPlaneIntersection(struct segmentListElement *segment) { return 0;}
static uint32_t renderViewportBorders(uint32_t currentVP) { return 0;}

Renderer::Renderer(QObject *parent) :
    QObject(parent)
{
}

bool Renderer::drawGUI() {
     return true;
}

bool Renderer::renderOrthogonalVP(uint32_t currentVP) {
    float dataPxX, dataPxY;

    if(!((state->viewerState->viewPorts[currentVP].type == VIEWPORT_XY)
            || (state->viewerState->viewPorts[currentVP].type == VIEWPORT_XZ)
            || (state->viewerState->viewPorts[currentVP].type == VIEWPORT_YZ))) {
        LOG("Wrong VP type given for renderOrthogonalVP() call.");
        return FALSE;
    }

    /* probably not needed TDitem
    glColor4f(0.5, 0.5, 0.5, 1.);
    glBegin(GL_QUADS);
        glVertex2d(0, 0);
        glVertex2d(state->viewerState->viewPorts[currentVP].edgeLength, 0);
        glVertex2d(state->viewerState->viewPorts[currentVP].edgeLength, state->viewerState->viewPorts[currentVP].edgeLength);
        glVertex2d(0, state->viewerState->viewPorts[currentVP].edgeLength);
    glEnd();
    */


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    //glClear(GL_DEPTH_BUFFER_BIT); /* better place? TDitem */

    if(!state->viewerState->selectModeFlag) {
        if(state->viewerState->multisamplingOnOff) glEnable(GL_MULTISAMPLE);

        if(state->viewerState->lightOnOff) {
            /* Configure light. optimize that! TDitem */
            glEnable(GL_LIGHTING);
            GLfloat ambientLight[] = {0.5, 0.5, 0.5, 0.8};
            GLfloat diffuseLight[] = {1., 1., 1., 1.};
            GLfloat lightPos[] = {0., 0., 1., 1.};

            glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
            glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
            glLightfv(GL_LIGHT0,GL_POSITION,lightPos);
            glEnable(GL_LIGHT0);

            GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

            /* Enable materials with automatic color assignment */
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
            glEnable(GL_COLOR_MATERIAL);
        }
    }

    /* Multiplying by state->magnification increases the area covered
     * by the textured OpenGL quad for downsampled datasets. */

    dataPxX = state->viewerState->viewPorts[currentVP].texture.displayedEdgeLengthX
            / state->viewerState->viewPorts[currentVP].texture.texUnitsPerDataPx
            * 0.5;
//            * (float)state->magnification;
    dataPxY = state->viewerState->viewPorts[currentVP].texture.displayedEdgeLengthY
            / state->viewerState->viewPorts[currentVP].texture.texUnitsPerDataPx
            * 0.5;
//            * (float)state->magnification;

    switch(state->viewerState->viewPorts[currentVP].type) {
        case VIEWPORT_XY:
            if(!state->viewerState->selectModeFlag) {
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
            }
            /* left, right, bottom, top, near, far clipping planes */
            glOrtho(-((float)(state->boundary.x)/ 2.) + (float)state->viewerState->currentPosition.x - dataPxX,
                -((float)(state->boundary.x) / 2.) + (float)state->viewerState->currentPosition.x + dataPxX,
                -((float)(state->boundary.y) / 2.) + (float)state->viewerState->currentPosition.y - dataPxY,
                -((float)(state->boundary.y) / 2.) + (float)state->viewerState->currentPosition.y + dataPxY,
                ((float)(state->boundary.z) / 2.) - state->viewerState->depthCutOff - (float)state->viewerState->currentPosition.z,
                ((float)(state->boundary.z) / 2.) + state->viewerState->depthCutOff - (float)state->viewerState->currentPosition.z);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            /*optimize that! TDitem */

            glTranslatef(-((float)state->boundary.x / 2.),
                        -((float)state->boundary.y / 2.),
                        -((float)state->boundary.z / 2.));

            glTranslatef((float)state->viewerState->currentPosition.x,
                        (float)state->viewerState->currentPosition.y,
                        (float)state->viewerState->currentPosition.z);

            glRotatef(180., 1.,0.,0.);

            glLoadName(3);

            glEnable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(1., 1., 1., 1.);

            glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.texHandle);
            glBegin(GL_QUADS);
                glNormal3i(0,0,1);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx, state->viewerState->viewPorts[currentVP].texture.texLUy);
                glVertex3f(-dataPxX, -dataPxY, 0.);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx, state->viewerState->viewPorts[currentVP].texture.texRUy);
                glVertex3f(dataPxX, -dataPxY, 0.);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx, state->viewerState->viewPorts[currentVP].texture.texRLy);
                glVertex3f(dataPxX, dataPxY, 0.);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx, state->viewerState->viewPorts[currentVP].texture.texLLy);
                glVertex3f(-dataPxX, dataPxY, 0.);
            glEnd();
            glBindTexture (GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);


            glTranslatef(-(float)state->viewerState->currentPosition.x, -(float)state->viewerState->currentPosition.y, -(float)state->viewerState->currentPosition.z);
            glTranslatef(((float)state->boundary.x / 2.),((float)state->boundary.y / 2.),((float)state->boundary.z / 2.));

            if(state->skeletonState->displayListSkeletonSlicePlaneVP) glCallList(state->skeletonState->displayListSkeletonSlicePlaneVP);

            glTranslatef(-((float)state->boundary.x / 2.),-((float)state->boundary.y / 2.),-((float)state->boundary.z / 2.));
            glTranslatef((float)state->viewerState->currentPosition.x, (float)state->viewerState->currentPosition.y, (float)state->viewerState->currentPosition.z);
            glLoadName(3);

            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(1., 1., 1., 0.6);

            glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.texHandle);
            glBegin(GL_QUADS);
                glNormal3i(0,0,1);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx, state->viewerState->viewPorts[currentVP].texture.texLUy);
                glVertex3f(-dataPxX, -dataPxY, 1.);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx, state->viewerState->viewPorts[currentVP].texture.texRUy);
                glVertex3f(dataPxX, -dataPxY, 1.);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx, state->viewerState->viewPorts[currentVP].texture.texRLy);
                glVertex3f(dataPxX, dataPxY, 1.);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx, state->viewerState->viewPorts[currentVP].texture.texLLy);
                glVertex3f(-dataPxX, dataPxY, 1.);
            glEnd();

            /* Draw the overlay textures */
            if(state->overlay) {
                if(state->viewerState->overlayVisible) {
                    glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.overlayHandle);
                    glBegin(GL_QUADS);
                        glNormal3i(0, 0, 1);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx,
                                     state->viewerState->viewPorts[currentVP].texture.texLUy);
                        glVertex3f(-dataPxX, -dataPxY, -0.1);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx,
                                     state->viewerState->viewPorts[currentVP].texture.texRUy);
                        glVertex3f(dataPxX, -dataPxY, -0.1);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx,
                                     state->viewerState->viewPorts[currentVP].texture.texRLy);
                        glVertex3f(dataPxX, dataPxY, -0.1);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx,
                                     state->viewerState->viewPorts[currentVP].texture.texLLy);
                        glVertex3f(-dataPxX, dataPxY, -0.1);
                    glEnd();
                }
            }

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_DEPTH_TEST);

            if(state->viewerState->drawVPCrosshairs) {
                glLineWidth(1.);
                glBegin(GL_LINES);
                    glColor4f(0., 1., 0., 0.3);
                    glVertex3f(-dataPxX, 0.5, -0.0001);
                    glVertex3f(dataPxX, 0.5, -0.0001);

                    glColor4f(0., 0., 1., 0.3);
                    glVertex3f(0.5, -dataPxY, -0.0001);
                    glVertex3f(0.5, dataPxY, -0.0001);
                glEnd();
            }

            break;

        case VIEWPORT_XZ:
            if(!state->viewerState->selectModeFlag) {
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
            }
            /* left, right, bottom, top, near, far clipping planes */
            glOrtho(-((float)state->boundary.x / 2.) + (float)state->viewerState->currentPosition.x - dataPxX,
                -((float)state->boundary.x / 2.) + (float)state->viewerState->currentPosition.x + dataPxX,
                -((float)state->boundary.y / 2.) + (float)state->viewerState->currentPosition.y - dataPxY,
                -((float)state->boundary.y / 2.) + (float)state->viewerState->currentPosition.y + dataPxY,
                ((float)state->boundary.z / 2.) - state->viewerState->depthCutOff - (float)state->viewerState->currentPosition.z,
                ((float)state->boundary.z / 2.) + state->viewerState->depthCutOff - (float)state->viewerState->currentPosition.z);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            /*optimize that! TDitem */

            glTranslatef(-((float)state->boundary.x / 2.),
                        -((float)state->boundary.y / 2.),
                        -((float)state->boundary.z / 2.));

            glTranslatef((float)state->viewerState->currentPosition.x,
                        (float)state->viewerState->currentPosition.y,
                        (float)state->viewerState->currentPosition.z);

            glRotatef(90., 1., 0., 0.);

            glLoadName(3);

            glEnable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);


            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(1., 1., 1., 1.);

            glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.texHandle);
            glBegin(GL_QUADS);
                glNormal3i(0,1,0);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx, state->viewerState->viewPorts[currentVP].texture.texLUy);
                glVertex3f(-dataPxX, 0., -dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx, state->viewerState->viewPorts[currentVP].texture.texRUy);
                glVertex3f(dataPxX, 0., -dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx, state->viewerState->viewPorts[currentVP].texture.texRLy);
                glVertex3f(dataPxX, 0., dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx, state->viewerState->viewPorts[currentVP].texture.texLLy);
                glVertex3f(-dataPxX, 0., dataPxY);
            glEnd();
            glBindTexture (GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);

            glTranslatef(-(float)state->viewerState->currentPosition.x, -(float)state->viewerState->currentPosition.y, -(float)state->viewerState->currentPosition.z);
            glTranslatef(((float)state->boundary.x / 2.),((float)state->boundary.y / 2.),((float)state->boundary.z / 2.));

            if(state->skeletonState->displayListSkeletonSlicePlaneVP) glCallList(state->skeletonState->displayListSkeletonSlicePlaneVP);

            glTranslatef(-((float)state->boundary.x / 2.),-((float)state->boundary.y / 2.),-((float)state->boundary.z / 2.));
            glTranslatef((float)state->viewerState->currentPosition.x, (float)state->viewerState->currentPosition.y, (float)state->viewerState->currentPosition.z);
            glLoadName(3);

            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(1., 1., 1., 0.6);

            glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.texHandle);
            glBegin(GL_QUADS);
                glNormal3i(0,1,0);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx, state->viewerState->viewPorts[currentVP].texture.texLUy);
                glVertex3f(-dataPxX, 0., -dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx, state->viewerState->viewPorts[currentVP].texture.texRUy);
                glVertex3f(dataPxX, -0., -dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx, state->viewerState->viewPorts[currentVP].texture.texRLy);
                glVertex3f(dataPxX, -0., dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx, state->viewerState->viewPorts[currentVP].texture.texLLy);
                glVertex3f(-dataPxX, -0., dataPxY);
            glEnd();

            /* Draw overlay */
            if(state->overlay) {
                if(state->viewerState->overlayVisible) {
                    glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.overlayHandle);
                    glBegin(GL_QUADS);
                        glNormal3i(0,1,0);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx,
                                     state->viewerState->viewPorts[currentVP].texture.texLUy);
                        glVertex3f(-dataPxX, 0.1, -dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx,
                                     state->viewerState->viewPorts[currentVP].texture.texRUy);
                        glVertex3f(dataPxX, 0.1, -dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx,
                                     state->viewerState->viewPorts[currentVP].texture.texRLy);
                        glVertex3f(dataPxX, 0.1, dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx,
                                     state->viewerState->viewPorts[currentVP].texture.texLLy);
                        glVertex3f(-dataPxX, 0.1, dataPxY);
                    glEnd();
                }
            }
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_DEPTH_TEST);

            if(state->viewerState->drawVPCrosshairs) {
                glLineWidth(1.);
                glBegin(GL_LINES);
                    glColor4f(1., 0., 0., 0.3);
                    glVertex3f(-dataPxX, 0.0001, 0.5);
                    glVertex3f(dataPxX, 0.0001, 0.5);

                    glColor4f(0., 0., 1., 0.3);
                    glVertex3f(0.5, 0.0001, -dataPxY);
                    glVertex3f(0.5, 0.0001, dataPxY);
                glEnd();
            }

            break;
        case VIEWPORT_YZ:
            if(!state->viewerState->selectModeFlag) {
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
            }

            glOrtho(-((float)state->boundary.x / 2.) + (float)state->viewerState->currentPosition.x - dataPxY,
                -((float)state->boundary.x / 2.) + (float)state->viewerState->currentPosition.x + dataPxY,
                -((float)state->boundary.y / 2.) + (float)state->viewerState->currentPosition.y - dataPxX,
                -((float)state->boundary.y / 2.) + (float)state->viewerState->currentPosition.y + dataPxX,
                ((float)state->boundary.z / 2.) - state->viewerState->depthCutOff - (float)state->viewerState->currentPosition.z,
                ((float)state->boundary.z / 2.) + state->viewerState->depthCutOff - (float)state->viewerState->currentPosition.z);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glTranslatef(-((float)state->boundary.x / 2.),-((float)state->boundary.y / 2.),-((float)state->boundary.z / 2.));
            glTranslatef((float)state->viewerState->currentPosition.x, (float)state->viewerState->currentPosition.y, (float)state->viewerState->currentPosition.z);
            glRotatef(90., 0., 1., 0.);
            glScalef(1., -1., 1.);

            glLoadName(3);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(1., 1., 1., 1.);

            glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.texHandle);
            glBegin(GL_QUADS);
                glNormal3i(1,0,0);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx, state->viewerState->viewPorts[currentVP].texture.texLUy);
                glVertex3f(0., -dataPxX, -dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx, state->viewerState->viewPorts[currentVP].texture.texRUy);
                glVertex3f(0., dataPxX, -dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx, state->viewerState->viewPorts[currentVP].texture.texRLy);
                glVertex3f(0., dataPxX, dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx, state->viewerState->viewPorts[currentVP].texture.texLLy);
                glVertex3f(0., -dataPxX, dataPxY);
            glEnd();
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);

            glTranslatef(-(float)state->viewerState->currentPosition.x, -(float)state->viewerState->currentPosition.y, -(float)state->viewerState->currentPosition.z);
            glTranslatef((float)state->boundary.x / 2.,(float)state->boundary.y / 2.,(float)state->boundary.z / 2.);

            if(state->skeletonState->displayListSkeletonSlicePlaneVP) glCallList(state->skeletonState->displayListSkeletonSlicePlaneVP);

            glTranslatef(-((float)state->boundary.x / 2.),-((float)state->boundary.y / 2.),-((float)state->boundary.z / 2.));
            glTranslatef((float)state->viewerState->currentPosition.x, (float)state->viewerState->currentPosition.y, (float)state->viewerState->currentPosition.z);
            glLoadName(3);

            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(1., 1., 1., 0.6);

            glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.texHandle);
            glBegin(GL_QUADS);
                glNormal3i(1,0,0);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx, state->viewerState->viewPorts[currentVP].texture.texLUy);
                glVertex3f(1., -dataPxX, -dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx, state->viewerState->viewPorts[currentVP].texture.texRUy);
                glVertex3f(1., dataPxX, -dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx, state->viewerState->viewPorts[currentVP].texture.texRLy);
                glVertex3f(1., dataPxX, dataPxY);
                glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx, state->viewerState->viewPorts[currentVP].texture.texLLy);
                glVertex3f(1., -dataPxX, dataPxY);
            glEnd();

            /* Draw overlay */
            if(state->overlay) {
                if(state->viewerState->overlayVisible) {
                    glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[currentVP].texture.overlayHandle);
                    glBegin(GL_QUADS);
                        glNormal3i(1,0,0);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLUx,
                                     state->viewerState->viewPorts[currentVP].texture.texLUy);
                        glVertex3f(-0.1, -dataPxX, -dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRUx,
                                     state->viewerState->viewPorts[currentVP].texture.texRUy);
                    glVertex3f(-0.1, dataPxX, -dataPxY);
                    glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texRLx,
                                 state->viewerState->viewPorts[currentVP].texture.texRLy);
                    glVertex3f(-0.1, dataPxX, dataPxY);
                    glTexCoord2f(state->viewerState->viewPorts[currentVP].texture.texLLx,
                                 state->viewerState->viewPorts[currentVP].texture.texLLy);
                    glVertex3f(-0.1, -dataPxX, dataPxY);
                    glEnd();
                }
            }

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_DEPTH_TEST);

            if(state->viewerState->drawVPCrosshairs) {
                glLineWidth(1.);
                glBegin(GL_LINES);
                    glColor4f(1., 0., 0., 0.3);
                    glVertex3f(-0.0001, -dataPxX, 0.5);
                    glVertex3f(-0.0001, dataPxX, 0.5);

                    glColor4f(0., 1., 0., 0.3);
                    glVertex3f(-0.0001, 0.5, -dataPxX);
                    glVertex3f(-0.0001, 0.5, dataPxX);
                glEnd();
            }

            break;
    }

    glDisable(GL_BLEND);
    renderViewportBorders(currentVP);

    return true;
}

bool Renderer::renderSkeletonVP(uint32_t currentVP) {
    char *textBuffer;
        char *c;
        uint32_t i;

        GLUquadricObj *gluCylObj = NULL;

        // Used for calculation of slice pane length inside the 3d view
        float dataPxX, dataPxY;

        //textBuffer = malloc(32);
        memset(textBuffer, '\0', 32);

        glClear(GL_DEPTH_BUFFER_BIT); // better place? TDitem

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // left, right, bottom, top, near, far clipping planes; substitute arbitrary vals to something more sensible. TDitem
    //LOG("%f, %f, %f", state->skeletonState->translateX, state->skeletonState->translateY, state->skeletonState->zoomLevel);
        glOrtho(state->skeletonState->volBoundary * state->skeletonState->zoomLevel + state->skeletonState->translateX,
            state->skeletonState->volBoundary - (state->skeletonState->volBoundary * state->skeletonState->zoomLevel) + state->skeletonState->translateX,
            state->skeletonState->volBoundary - (state->skeletonState->volBoundary * state->skeletonState->zoomLevel) + state->skeletonState->translateY,
            state->skeletonState->volBoundary * state->skeletonState->zoomLevel + state->skeletonState->translateY, -1000, 10 *state->skeletonState->volBoundary);

        if(state->viewerState->lightOnOff) {
            // Configure light
            glEnable(GL_LIGHTING);
            GLfloat ambientLight[] = {0.5, 0.5, 0.5, 0.8};
            GLfloat diffuseLight[] = {1., 1., 1., 1.};
            GLfloat lightPos[] = {0., 0., 1., 1.};

            glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
            glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
            glLightfv(GL_LIGHT0,GL_POSITION,lightPos);
            glEnable(GL_LIGHT0);

            GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

            // Enable materials with automatic color tracking
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
            glEnable(GL_COLOR_MATERIAL);
        }
        if(state->viewerState->multisamplingOnOff) glEnable(GL_MULTISAMPLE);


         // Now we set up the view on the skeleton and draw some very basic VP stuff like the gray background

        state->skeletonState->viewChanged = TRUE;
        if(state->skeletonState->viewChanged) {
            state->skeletonState->viewChanged = FALSE;
            if(state->skeletonState->displayListView) glDeleteLists(state->skeletonState->displayListView, 1);
            state->skeletonState->displayListView = glGenLists(1);
            // COMPILE_AND_EXECUTE because we grab the rotation matrix inside!
            glNewList(state->skeletonState->displayListView, GL_COMPILE_AND_EXECUTE);

            glEnable(GL_DEPTH_TEST);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

             // Now we draw the  background of our skeleton VP


            glPushMatrix();
            glTranslatef(0., 0., -10. * ((float)state->skeletonState->volBoundary - 2.));

            glShadeModel(GL_SMOOTH);
            glDisable(GL_TEXTURE_2D);

            glLoadName(1);
            glColor4f(0.9, 0.9, 0.9, 1.);
            // The * 10 should prevent, that the user translates into space with gray background - dirty solution. TDitem
            glBegin(GL_QUADS);
                glVertex3i(-state->skeletonState->volBoundary * 10, -state->skeletonState->volBoundary * 10, 0);
                glVertex3i(state->skeletonState->volBoundary  * 10, -state->skeletonState->volBoundary * 10, 0);
                glVertex3i(state->skeletonState->volBoundary  * 10, state->skeletonState->volBoundary  * 10, 0);
                glVertex3i(-state->skeletonState->volBoundary * 10, state->skeletonState->volBoundary  * 10, 0);
            glEnd();

            glPopMatrix();

            // load model view matrix that stores rotation state!
            glLoadMatrixf(state->skeletonState->skeletonVpModelView);


            // perform user defined coordinate system rotations. use single matrix multiplication as opt.! TDitem
            if((state->skeletonState->rotdx)
                || (state->skeletonState->rotdy)
                ){


                if((state->skeletonState->rotateAroundActiveNode) && (state->skeletonState->activeNode)) {
                    glTranslatef(-((float)state->boundary.x / 2.),-((float)state->boundary.y / 2),-((float)state->boundary.z / 2.));
                    glTranslatef((float)state->skeletonState->activeNode->position.x,
                                 (float)state->skeletonState->activeNode->position.y,
                                 (float)state->skeletonState->activeNode->position.z);
                    glScalef(1., 1., state->viewerState->voxelXYtoZRatio);
                    rotateSkeletonViewport();
                    glScalef(1., 1., 1./state->viewerState->voxelXYtoZRatio);
                    glTranslatef(-(float)state->skeletonState->activeNode->position.x,
                                 -(float)state->skeletonState->activeNode->position.y,
                                 -(float)state->skeletonState->activeNode->position.z);
                    glTranslatef(((float)state->boundary.x / 2.),((float)state->boundary.y / 2.),((float)state->boundary.z / 2.));
                }
                // rotate around dataset center if no active node is selected
                else {
                    glScalef(1., 1., state->viewerState->voxelXYtoZRatio);
                    rotateSkeletonViewport();
                    glScalef(1., 1., 1./state->viewerState->voxelXYtoZRatio);
                }

                // save the modified basic model view matrix

                glGetFloatv(GL_MODELVIEW_MATRIX, state->skeletonState->skeletonVpModelView);

                // reset the relative rotation angles because rotation has been performed.
            }

            switch(state->skeletonState->definedSkeletonVpView) {
                case 0:
                    break;
                case 1:
                    // XY viewport like view
                    state->skeletonState->definedSkeletonVpView = 0;

                    glLoadIdentity();
                    glTranslatef((float)state->skeletonState->volBoundary / 2.,
                                 (float)state->skeletonState->volBoundary / 2.,
                                 (float)state->skeletonState->volBoundary / -2.);
    //glScalef(1., 1., 1./state->viewerState->voxelXYtoZRatio);
                    glGetFloatv(GL_MODELVIEW_MATRIX, state->skeletonState->skeletonVpModelView);

                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();

                    state->skeletonState->translateX = ((float)state->boundary.x / -2.) + (float)state->viewerState->currentPosition.x;
                    state->skeletonState->translateY = ((float)state->boundary.y / -2.) + (float)state->viewerState->currentPosition.y;

                    glOrtho(state->skeletonState->volBoundary * state->skeletonState->zoomLevel + state->skeletonState->translateX,
                            state->skeletonState->volBoundary - (state->skeletonState->volBoundary * state->skeletonState->zoomLevel) + state->skeletonState->translateX,
                            state->skeletonState->volBoundary - (state->skeletonState->volBoundary * state->skeletonState->zoomLevel) + state->skeletonState->translateY,
                            state->skeletonState->volBoundary * state->skeletonState->zoomLevel + state->skeletonState->translateY,
                            -500,
                            10 * state->skeletonState->volBoundary);
                    setRotationState(ROTATIONSTATEXY);
                    break;

                case 2:
                    // XZ viewport like view
                    state->skeletonState->definedSkeletonVpView = 0;

                    glLoadIdentity();

                    glTranslatef((float)state->skeletonState->volBoundary / 2.,
                                 (float)state->skeletonState->volBoundary / 2.,
                                 (float)state->skeletonState->volBoundary / -2.);

                    glRotatef(90, 0., 1., 0.);
                    glScalef(1., 1., 1./state->viewerState->voxelXYtoZRatio);
                    glGetFloatv(GL_MODELVIEW_MATRIX, state->skeletonState->skeletonVpModelView);

                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();

                    state->skeletonState->translateX = ((float)state->boundary.z / -2.) + (float)state->viewerState->currentPosition.z;
                    state->skeletonState->translateY = ((float)state->boundary.y / -2.) + (float)state->viewerState->currentPosition.y;

                    glOrtho(state->skeletonState->volBoundary * state->skeletonState->zoomLevel + state->skeletonState->translateX,
                            state->skeletonState->volBoundary - (state->skeletonState->volBoundary * state->skeletonState->zoomLevel) + state->skeletonState->translateX,
                            state->skeletonState->volBoundary - (state->skeletonState->volBoundary * state->skeletonState->zoomLevel) + state->skeletonState->translateY,
                            state->skeletonState->volBoundary * state->skeletonState->zoomLevel + state->skeletonState->translateY,
                            -500,
                            10 * state->skeletonState->volBoundary);
                    setRotationState(ROTATIONSTATEXZ);
                    break;

                case 3:
                    // YZ viewport like view
                    state->skeletonState->definedSkeletonVpView = 0;
                    glLoadIdentity();
                    glTranslatef((float)state->skeletonState->volBoundary / 2.,
                                 (float)state->skeletonState->volBoundary / 2.,
                                 (float)state->skeletonState->volBoundary / -2.);
                    glRotatef(270, 1., 0., 0.);
                    glScalef(1., 1., 1./state->viewerState->voxelXYtoZRatio);

                    glGetFloatv(GL_MODELVIEW_MATRIX, state->skeletonState->skeletonVpModelView);

                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();

                    state->skeletonState->translateX = ((float)state->boundary.x / -2.) + (float)state->viewerState->currentPosition.x;
                    state->skeletonState->translateY = ((float)state->boundary.z / -2.) + (float)state->viewerState->currentPosition.z;

                    glOrtho(state->skeletonState->volBoundary * state->skeletonState->zoomLevel + state->skeletonState->translateX,
                            state->skeletonState->volBoundary - (state->skeletonState->volBoundary * state->skeletonState->zoomLevel) + state->skeletonState->translateX,
                            state->skeletonState->volBoundary - (state->skeletonState->volBoundary * state->skeletonState->zoomLevel) + state->skeletonState->translateY,
                            state->skeletonState->volBoundary * state->skeletonState->zoomLevel + state->skeletonState->translateY,
                            -500,
                            10 * state->skeletonState->volBoundary);
                    setRotationState(ROTATIONSTATEYZ);
                    break;
                //float minrotation[16];
                //float da;
                case 4:
                //90deg
                    state->skeletonState->rotdx = 10;
                    state->skeletonState->rotationcounter++;
                    if (state->skeletonState->rotationcounter > 15) {
                        state->skeletonState->rotdx = 7.6;
                        state->skeletonState->definedSkeletonVpView = 0;
                        state->skeletonState->rotationcounter = 0;
                    }
                    break;

                case 5:
                //180deg
                    state->skeletonState->rotdx = 10;
                    state->skeletonState->rotationcounter++;
                    if (state->skeletonState->rotationcounter > 31) {
                        state->skeletonState->rotdx = 5.2;
                        state->skeletonState->definedSkeletonVpView = 0;
                        state->skeletonState->rotationcounter = 0;
                    }
                    break;
                case 6:
                    // Resetting
                    state->skeletonState->definedSkeletonVpView = 0;
                    state->skeletonState->translateX = 0;
                    state->skeletonState->translateY = 0;
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    glTranslatef((float)state->skeletonState->volBoundary / 2.,
                                 (float)state->skeletonState->volBoundary / 2.,
                                 (float)state->skeletonState->volBoundary / -2.);
                    glScalef(-1., 1., 1.);
                    glRotatef(235., 1., 0., 0.);
                    glRotatef(210., 0., 0., 1.);
                    glGetFloatv(GL_MODELVIEW_MATRIX, state->skeletonState->skeletonVpModelView);
                    state->skeletonState->zoomLevel = SKELZOOMMIN;
                    setRotationState(ROTATIONSTATERESET);
                    break;
            }


             // Draw the slice planes for orientation inside the data stack


            glPushMatrix();

            // single operation! TDitem
            glTranslatef(-((float)state->boundary.x / 2.),-((float)state->boundary.y / 2.),-((float)state->boundary.z / 2.));
            glTranslatef(0.5,0.5,0.5);
            glTranslatef((float)state->viewerState->currentPosition.x, (float)state->viewerState->currentPosition.y, (float)state->viewerState->currentPosition.z);

            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(1., 1., 1., 1.);

            for(i = 0; i < state->viewerState->numberViewPorts; i++) {
                dataPxX = state->viewerState->viewPorts[i].texture.displayedEdgeLengthX
                        / state->viewerState->viewPorts[i].texture.texUnitsPerDataPx
                        * 0.5;
                dataPxY = state->viewerState->viewPorts[i].texture.displayedEdgeLengthY
                    / state->viewerState->viewPorts[i].texture.texUnitsPerDataPx
                    * 0.5;

                switch(state->viewerState->viewPorts[i].type) {
                case VIEWPORT_XY:
                    if(!state->skeletonState->showXYplane) break;

                    glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[i].texture.texHandle);
                    glLoadName(VIEWPORT_XY);
                    glBegin(GL_QUADS);
                        glNormal3i(0,0,1);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texLUx, state->viewerState->viewPorts[i].texture.texLUy);
                        glVertex3f(-dataPxX, -dataPxY, 0.);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texRUx, state->viewerState->viewPorts[i].texture.texRUy);
                        glVertex3f(dataPxX, -dataPxY, 0.);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texRLx, state->viewerState->viewPorts[i].texture.texRLy);
                        glVertex3f(dataPxX, dataPxY, 0.);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texLLx, state->viewerState->viewPorts[i].texture.texLLy);
                        glVertex3f(-dataPxX, dataPxY, 0.);
                    glEnd();
                    glBindTexture (GL_TEXTURE_2D, 0);
                    break;
                case VIEWPORT_XZ:
                    if(!state->skeletonState->showXZplane) break;
                    glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[i].texture.texHandle);
                    glLoadName(VIEWPORT_XZ);
                    glBegin(GL_QUADS);
                        glNormal3i(0,1,0);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texLUx, state->viewerState->viewPorts[i].texture.texLUy);
                        glVertex3f(-dataPxX, 0., -dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texRUx, state->viewerState->viewPorts[i].texture.texRUy);
                        glVertex3f(dataPxX, 0., -dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texRLx, state->viewerState->viewPorts[i].texture.texRLy);
                        glVertex3f(dataPxX, 0., dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texLLx, state->viewerState->viewPorts[i].texture.texLLy);
                        glVertex3f(-dataPxX, 0., dataPxY);
                    glEnd();
                    glBindTexture (GL_TEXTURE_2D, 0);
                    break;
                case VIEWPORT_YZ:
                    if(!state->skeletonState->showYZplane) break;
                    glBindTexture(GL_TEXTURE_2D, state->viewerState->viewPorts[i].texture.texHandle);
                    glLoadName(VIEWPORT_YZ);
                    glBegin(GL_QUADS);
                        glNormal3i(1,0,0);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texLUx, state->viewerState->viewPorts[i].texture.texLUy);
                        glVertex3f(0., -dataPxX, -dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texRUx, state->viewerState->viewPorts[i].texture.texRUy);
                        glVertex3f(0., dataPxX, -dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texRLx, state->viewerState->viewPorts[i].texture.texRLy);
                        glVertex3f(0., dataPxX, dataPxY);
                        glTexCoord2f(state->viewerState->viewPorts[i].texture.texLLx, state->viewerState->viewPorts[i].texture.texLLy);
                        glVertex3f(0., -dataPxX, dataPxY);
                    glEnd();
                    glBindTexture (GL_TEXTURE_2D, 0);
                    break;
                }

            }

            glDisable(GL_TEXTURE_2D);
            glLoadName(3);
            for(i = 0; i < state->viewerState->numberViewPorts; i++) {
                dataPxX = state->viewerState->viewPorts[i].texture.displayedEdgeLengthX
                    / state->viewerState->viewPorts[i].texture.texUnitsPerDataPx
                    * 0.5;
                dataPxY = state->viewerState->viewPorts[i].texture.displayedEdgeLengthY
                    / state->viewerState->viewPorts[i].texture.texUnitsPerDataPx
                    * 0.5;
                switch(state->viewerState->viewPorts[i].type) {
                case VIEWPORT_XY:
                    glColor4f(0.7, 0., 0., 1.);
                    glBegin(GL_LINE_LOOP);
                        glVertex3f(-dataPxX, -dataPxY, 0.);
                        glVertex3f(dataPxX, -dataPxY, 0.);
                        glVertex3f(dataPxX, dataPxY, 0.);
                        glVertex3f(-dataPxX, dataPxY, 0.);
                    glEnd();

                    glColor4f(0., 0., 0., 1.);
                    glPushMatrix();
                    glTranslatef(-dataPxX, 0., 0.);
                    glRotatef(90., 0., 1., 0.);
                    gluCylObj = gluNewQuadric();
                    gluQuadricNormals(gluCylObj, GLU_SMOOTH);
                    gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
                    gluCylinder(gluCylObj, 0.4, 0.4, dataPxX * 2, 5, 5);
                    gluDeleteQuadric(gluCylObj);
                    glPopMatrix();

                    glPushMatrix();
                    glTranslatef(0., dataPxY, 0.);
                    glRotatef(90., 1., 0., 0.);
                    gluCylObj = gluNewQuadric();
                    gluQuadricNormals(gluCylObj, GLU_SMOOTH);
                    gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
                    gluCylinder(gluCylObj, 0.4, 0.4, dataPxY * 2, 5, 5);
                    gluDeleteQuadric(gluCylObj);
                    glPopMatrix();

                    break;
                case VIEWPORT_XZ:
                    glColor4f(0., 0.7, 0., 1.);
                    glBegin(GL_LINE_LOOP);
                        glVertex3f(-dataPxX, 0., -dataPxY);
                        glVertex3f(dataPxX, 0., -dataPxY);
                        glVertex3f(dataPxX, 0., dataPxY);
                        glVertex3f(-dataPxX, 0., dataPxY);
                    glEnd();

                    glColor4f(0., 0., 0., 1.);
                    glPushMatrix();
                    glTranslatef(0., 0., -dataPxY);
                    gluCylObj = gluNewQuadric();
                    gluQuadricNormals(gluCylObj, GLU_SMOOTH);
                    gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
                    gluCylinder(gluCylObj, 0.4, 0.4, dataPxY * 2, 5, 5);
                    gluDeleteQuadric(gluCylObj);
                    glPopMatrix();

                    break;
                case VIEWPORT_YZ:
                    glColor4f(0., 0., 0.7, 1.);
                    glBegin(GL_LINE_LOOP);
                        glVertex3f(0., -dataPxX, -dataPxY);
                        glVertex3f(0., dataPxX, -dataPxY);
                        glVertex3f(0., dataPxX, dataPxY);
                        glVertex3f(0., -dataPxX, dataPxY);
                    glEnd();
                    break;
                }
            }

            glPopMatrix();
            glEndList();
        }
        else {
            glCallList(state->skeletonState->displayListView);
        }


         // Now we draw the skeleton structure (Changes of it are adressed inside updateSkeletonDisplayList())

        if(state->skeletonState->displayListSkeletonSkeletonizerVP)
            glCallList(state->skeletonState->displayListSkeletonSkeletonizerVP);


         // Now we draw the dataset corresponding stuff (volume box of right size, axis descriptions...)


        if(state->skeletonState->datasetChanged) {

            state->skeletonState->datasetChanged = FALSE;
            if(state->skeletonState->displayListDataset) glDeleteLists(state->skeletonState->displayListDataset, 1);
            state->skeletonState->displayListDataset = glGenLists(1);
            glNewList(state->skeletonState->displayListDataset, GL_COMPILE);
            glEnable(GL_BLEND);


             // Now we draw the data volume box. use display list for that...very static TDitem


            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLoadName(3);
            glColor4f(0., 0., 0., 0.1);
            glBegin(GL_QUADS);
                glNormal3i(0,0,1);
                glVertex3i(-(state->boundary.x / 2), -(state->boundary.y / 2), -(state->boundary.z / 2));
                glVertex3i(state->boundary.x / 2, -(state->boundary.y / 2), -(state->boundary.z / 2));

                glVertex3i(state->boundary.x / 2, (state->boundary.y / 2), -(state->boundary.z / 2));
                glVertex3i(-(state->boundary.x / 2), (state->boundary.y / 2), -(state->boundary.z / 2));

                glNormal3i(0,0,1);
                glVertex3i(-(state->boundary.x / 2), -(state->boundary.y / 2), (state->boundary.z / 2));
                glVertex3i(state->boundary.x / 2, -(state->boundary.y / 2), (state->boundary.z / 2));

                glVertex3i(state->boundary.x / 2, (state->boundary.y / 2), (state->boundary.z / 2));
                glVertex3i(-(state->boundary.x / 2), (state->boundary.y / 2), (state->boundary.z / 2));

                glNormal3i(0,1,0);
                glVertex3i(-(state->boundary.x / 2), -(state->boundary.y / 2), -(state->boundary.z / 2));
                glVertex3i(-(state->boundary.x / 2), -(state->boundary.y / 2), (state->boundary.z / 2));

                glVertex3i(state->boundary.x / 2, -(state->boundary.y / 2), (state->boundary.z / 2));
                glVertex3i(state->boundary.x / 2, -(state->boundary.y / 2), -(state->boundary.z / 2));

                glNormal3i(0,1,0);
                glVertex3i(-(state->boundary.x / 2), (state->boundary.y / 2), -(state->boundary.z / 2));
                glVertex3i(-(state->boundary.x / 2), (state->boundary.y / 2), (state->boundary.z / 2));

                glVertex3i(state->boundary.x / 2, (state->boundary.y / 2), (state->boundary.z / 2));
                glVertex3i(state->boundary.x / 2, (state->boundary.y / 2), -(state->boundary.z / 2));

                glNormal3i(1,0,0);
                glVertex3i(-(state->boundary.x / 2), -(state->boundary.y / 2), -(state->boundary.z / 2));
                glVertex3i(-(state->boundary.x / 2), -(state->boundary.y / 2), (state->boundary.z / 2));

                glVertex3i(-(state->boundary.x / 2), (state->boundary.y / 2), (state->boundary.z / 2));
                glVertex3i(-(state->boundary.x / 2), (state->boundary.y / 2), -(state->boundary.z / 2));

                glNormal3i(1,0,0);
                glVertex3i(state->boundary.x / 2, -(state->boundary.y / 2), -(state->boundary.z / 2));
                glVertex3i(state->boundary.x / 2, -(state->boundary.y / 2), (state->boundary.z / 2));

                glVertex3i(state->boundary.x / 2, (state->boundary.y / 2), (state->boundary.z / 2));
                glVertex3i(state->boundary.x / 2, (state->boundary.y / 2), -(state->boundary.z / 2));
            glEnd();

            glColor4f(0., 0., 0., 1.);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glPushMatrix();
            glTranslatef(-(state->boundary.x / 2),-(state->boundary.y / 2),-(state->boundary.z / 2));
            gluCylObj = gluNewQuadric();
            gluQuadricNormals(gluCylObj, GLU_SMOOTH);
            gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
            gluCylinder(gluCylObj, 3., 3. , state->boundary.z, 5, 5);
            gluDeleteQuadric(gluCylObj);

            glPushMatrix();
            glTranslatef(0.,0., state->boundary.z);
            gluCylObj = gluNewQuadric();
            gluQuadricNormals(gluCylObj, GLU_SMOOTH);
            gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
            gluCylinder(gluCylObj, 15., 0. , 50., 15, 15);
            gluDeleteQuadric(gluCylObj);
            glPopMatrix();

            gluCylObj = gluNewQuadric();
            gluQuadricNormals(gluCylObj, GLU_SMOOTH);
            gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
            glRotatef(90., 0., 1., 0.);
            gluCylinder(gluCylObj, 3., 3. , state->boundary.x, 5, 5);
            gluDeleteQuadric(gluCylObj);

            glPushMatrix();
            glTranslatef(0.,0., state->boundary.x);
            gluCylObj = gluNewQuadric();
            gluQuadricNormals(gluCylObj, GLU_SMOOTH);
            gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
            gluCylinder(gluCylObj, 15., 0. , 50., 15, 15);
            gluDeleteQuadric(gluCylObj);
            glPopMatrix();

            gluCylObj = gluNewQuadric();
            gluQuadricNormals(gluCylObj, GLU_SMOOTH);
            gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
            glRotatef(-90., 1., 0., 0.);
            gluCylinder(gluCylObj, 3., 3. , state->boundary.y, 5, 5);
            gluDeleteQuadric(gluCylObj);

            glPushMatrix();
            glTranslatef(0.,0., state->boundary.y);
            gluCylObj = gluNewQuadric();
            gluQuadricNormals(gluCylObj, GLU_SMOOTH);
            gluQuadricOrientation(gluCylObj, GLU_OUTSIDE);
            gluCylinder(gluCylObj, 15., 0. , 50., 15, 15);
            gluDeleteQuadric(gluCylObj);
            glPopMatrix();

            glPopMatrix();


             // Draw axis description


            glColor4f(0., 0., 0., 1.);
            memset(textBuffer, '\0', 32);
            glRasterPos3f((float)-(state->boundary.x) / 2. - 50., (float)-(state->boundary.y) / 2. - 50., (float)-(state->boundary.z) / 2. - 50.);
            sprintf(textBuffer, "1, 1, 1");
            for (c=textBuffer; *c != '\0'; c++) {
                //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);
            }
            memset(textBuffer, '\0', 32);
            glRasterPos3f((float)(state->boundary.x) / 2. - 50., -(state->boundary.y / 2) - 50., -(state->boundary.z / 2)- 50.);
            sprintf(textBuffer, "%d, 1, 1", state->boundary.x + 1);
            for (c=textBuffer; *c != '\0'; c++) {
               // glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);
            }

            memset(textBuffer, '\0', 32);
            glRasterPos3f(-(state->boundary.x / 2)- 50., (float)(state->boundary.y) / 2. - 50., -(state->boundary.z / 2)- 50.);
            sprintf(textBuffer, "1, %d, 1", state->boundary.y + 1);
            for (c=textBuffer; *c != '\0'; c++) {
                //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);
            }
            memset(textBuffer, '\0', 32);
            glRasterPos3f(-(state->boundary.x / 2)- 50., -(state->boundary.y / 2)- 50., (float)(state->boundary.z) / 2. - 50.);
            sprintf(textBuffer, "1, 1, %d", state->boundary.z + 1);
            for (c=textBuffer; *c != '\0'; c++) {
                //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);
            }
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            glEndList();
            glCallList(state->skeletonState->displayListDataset);

        }
        else {
            glCallList(state->skeletonState->displayListDataset);
        }


         // Reset previously changed OGL parameters


        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glDisable(GL_MULTISAMPLE);
        glLoadIdentity();

        free(textBuffer);

        renderViewportBorders(currentVP);

        return TRUE;
}

uint32_t Renderer::retrieveVisibleObjectBeneathSquare(uint32_t currentVP, uint32_t x, uint32_t y, uint32_t width) {
        uint32_t i;
        /* 8192 is really arbitrary. It should be a value dependent on the
        number of nodes / segments */
        GLuint selectionBuffer[8192] = {0};
        GLint hits, openGLviewport[4];
        GLuint names, *ptr, minZ, *ptrName;
        ptrName = NULL;

        glViewport(state->viewerState->viewPorts[currentVP].upperLeftCorner.x,
            state->viewerState->screenSizeY
            - state->viewerState->viewPorts[currentVP].upperLeftCorner.y
            - state->viewerState->viewPorts[currentVP].edgeLength,
            state->viewerState->viewPorts[currentVP].edgeLength,
            state->viewerState->viewPorts[currentVP].edgeLength);

        glGetIntegerv(GL_VIEWPORT, openGLviewport);

        glSelectBuffer(8192, selectionBuffer);

        state->viewerState->selectModeFlag = TRUE;

        glRenderMode(GL_SELECT);

        glInitNames();
        glPushName(0);

        glMatrixMode(GL_PROJECTION);

        glLoadIdentity();

        gluPickMatrix(x, y, (float)width, (float)width, openGLviewport);

        if(state->viewerState->viewPorts[currentVP].type == VIEWPORT_SKELETON) {
                glOrtho(state->skeletonState->volBoundary
                    * state->skeletonState->zoomLevel
                    + state->skeletonState->translateX,
                    state->skeletonState->volBoundary
                    - (state->skeletonState->volBoundary
                    * state->skeletonState->zoomLevel)
                    + state->skeletonState->translateX,
                    state->skeletonState->volBoundary
                    - (state->skeletonState->volBoundary
                    * state->skeletonState->zoomLevel)
                    + state->skeletonState->translateY,
                    state->skeletonState->volBoundary
                    * state->skeletonState->zoomLevel
                    + state->skeletonState->translateY,
                    -10000, 10 * state->skeletonState->volBoundary);
                glCallList(state->skeletonState->displayListView);
                glCallList(state->skeletonState->displayListSkeletonSkeletonizerVP);
                glCallList(state->skeletonState->displayListDataset); //TDitem fix that display list !!

                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDisable(GL_BLEND);
                glDisable(GL_LIGHTING);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_MULTISAMPLE);
        }
        else {
                //glEnable(GL_DEPTH_TEST);
                //glCallList(state->viewerState->viewPorts[currentVP].displayList);
                glDisable(GL_DEPTH_TEST);
                renderOrthogonalVP(currentVP);
        }


        hits = glRenderMode(GL_RENDER);
        glLoadIdentity();

        ptr = (GLuint *)selectionBuffer;

        minZ = 0xffffffff;


        for(i = 0; i < hits; i++) {
            names = *ptr;

            ptr++;
            if((*ptr < minZ) && (*(ptr + 2) >= 50)) {
                minZ = *ptr;
                ptrName = ptr + 2;
            }
            ptr += names + 2;
        }

        state->viewerState->selectModeFlag = FALSE;
        if(ptrName) return *ptrName - 50;
        else return FALSE;
}

//Some math helper functions
float Renderer::radToDeg(float rad) {
    return ((180. * rad) / PI);
}

float Renderer::degToRad(float deg) {
    return ((deg / 180.) * PI);
}

float Renderer::scalarProduct(floatCoordinate *v1, floatCoordinate *v2) {
    return ((v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z));
}

floatCoordinate* Renderer::crossProduct(floatCoordinate *v1, floatCoordinate *v2) {
    floatCoordinate *result = NULL;
    //result = malloc(sizeof(floatCoordinate));
    result->x = v1->y * v2->z - v1->z * v2->y;
    result->y = v1->z * v2->x - v1->x * v2->z;
    result->z = v1->x * v2->y - v1->y * v2->x;
    return result;
}

float Renderer::vectorAngle(floatCoordinate *v1, floatCoordinate *v2) {
    return ((float)acos((double)(scalarProduct(v1, v2)) / (euclidicNorm(v1)*euclidicNorm(v2))));
}

float Renderer::euclidicNorm(floatCoordinate *v) {
    return ((float)sqrt((double)scalarProduct(v, v)));
}

bool Renderer::normalizeVector(floatCoordinate *v) {
    float norm = euclidicNorm(v);
    v->x /= norm;
    v->y /= norm;
    v->z /= norm;
    return true;
}

int32_t Renderer::roundFloat(float number) {
    if(number >= 0) return (int32_t)(number + 0.5);
    else return (int32_t)(number - 0.5);
}

int32_t Renderer::sgn(float number) {
    if(number > 0.) return 1;
    else if(number == 0.) return 0;
    else return -1;
}

bool Renderer::initRenderer() {
    /* initialize the textures used to display the SBFSEM data TDitem: return val check*/
    Viewer::initializeTextures();
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    /* Initialize the basic model view matrix for the skeleton VP
    Perform basic coordinate system rotations */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef((float)state->skeletonState->volBoundary / 2.,
        (float)state->skeletonState->volBoundary / 2.,
        -((float)state->skeletonState->volBoundary / 2.));

    glScalef(-1., 1., 1.);
    //);
    //LOG("state->viewerState->voxelXYtoZRatio = %f", state->viewerState->voxelXYtoZRatio);
    glRotatef(235., 1., 0., 0.);
    glRotatef(210., 0., 0., 1.);
    setRotationState(ROTATIONSTATERESET);
    //glScalef(1., 1., 1./state->viewerState->voxelXYtoZRatio);
    /* save the matrix for further use... */
    glGetFloatv(GL_MODELVIEW_MATRIX, state->skeletonState->skeletonVpModelView);

    glLoadIdentity();

    return true;
}

bool Renderer::splashScreen() {
    // Do we need this function? SplashScreen already implemented...
    return true;
}

bool Renderer::updateRotationStateMatrix(float M1[16], float M2[16]){
    //multiply matrix m2 to matrix m1 and save result in rotationState matrix
    int i;
    float M3[16];

    M3[0] = M1[0] * M2[0] + M1[4] * M2[1] + M1[8] * M2[2] + M1[12] * M2[3];
    M3[1] = M1[1] * M2[0] + M1[5] * M2[1] + M1[9] * M2[2] + M1[13] * M2[3];
    M3[2] = M1[2] * M2[0] + M1[6] * M2[1] + M1[10] * M2[2] + M1[14] * M2[3];
    M3[3] = M1[3] * M2[0] + M1[7] * M2[1] + M1[11] * M2[2] + M1[15] * M2[3];
    M3[4] = M1[0] * M2[4] + M1[4] * M2[5] + M1[8] * M2[6] + M1[12] * M2[7];
    M3[5] = M1[1] * M2[4] + M1[5] * M2[5] + M1[9] * M2[6] + M1[13] * M2[7];
    M3[6] = M1[2] * M2[4] + M1[6] * M2[5] + M1[10] * M2[6] + M1[14] * M2[7];
    M3[7] = M1[3] * M2[4] + M1[7] * M2[5] + M1[11] * M2[6] + M1[15] * M2[7];
    M3[8] = M1[0] * M2[8] + M1[4] * M2[9] + M1[8] * M2[10] + M1[12] * M2[11];
    M3[9] = M1[1] * M2[8] + M1[5] * M2[9] + M1[9] * M2[10] + M1[13] * M2[11];
    M3[10] = M1[2] * M2[8] + M1[6] * M2[9] + M1[10] * M2[10] + M1[14] * M2[11];
    M3[11] = M1[3] * M2[8] + M1[7] * M2[9] + M1[11] * M2[10] + M1[15] * M2[11];
    M3[12] = M1[0] * M2[12] + M1[4] * M2[13] + M1[8] * M2[14] + M1[12] * M2[15];
    M3[13] = M1[1] * M2[12] + M1[5] * M2[13] + M1[9] * M2[14] + M1[13] * M2[15];
    M3[14] = M1[2] * M2[12] + M1[6] * M2[13] + M1[10] * M2[14] + M1[14] * M2[15];
    M3[15] = M1[3] * M2[12] + M1[7] * M2[13] + M1[11] * M2[14] + M1[15] * M2[15];

    for (i = 0; i < 16; i++){
        state->skeletonState->rotationState[i] = M3[i];
    }
    return true;
}

bool Renderer::rotateSkeletonViewport(){

    // for general information look at http://de.wikipedia.org/wiki/Rolling-Ball-Rotation

    // rotdx and rotdy save the small rotations the user creates with one single mouse action
    // singleRotM[16] is the rotation matrix for this single mouse action (see )
    // state->skeletonstate->rotationState is the product of all rotations during KNOSSOS session
    // inverseRotationState is the inverse (here transposed) matrix of state->skeletonstate->rotationState

    float singleRotM[16];
    float inverseRotationState[16];
    float rotR = 100.;
    float rotdx = (float)state->skeletonState->rotdx;
    float rotdy = (float)state->skeletonState->rotdy;
    state->skeletonState->rotdx = 0;
    state->skeletonState->rotdy = 0;
    float rotdr = pow(rotdx * rotdx + rotdy * rotdy, 0.5);
    float rotCosT = rotR / (pow(rotR * rotR + rotdr * rotdr, 0.5));
    float rotSinT = rotdr / (pow(rotR * rotR + rotdr * rotdr, 0.5));

    //calc inverse matrix of actual rotation state
    inverseRotationState[0] = state->skeletonState->rotationState[0];
    inverseRotationState[1] = state->skeletonState->rotationState[4];
    inverseRotationState[2] = state->skeletonState->rotationState[8];
    inverseRotationState[3] = state->skeletonState->rotationState[12];
    inverseRotationState[4] = state->skeletonState->rotationState[1];
    inverseRotationState[5] = state->skeletonState->rotationState[5];
    inverseRotationState[6] = state->skeletonState->rotationState[9];
    inverseRotationState[7] = state->skeletonState->rotationState[13];
    inverseRotationState[8] = state->skeletonState->rotationState[2];
    inverseRotationState[9] = state->skeletonState->rotationState[6];
    inverseRotationState[10] = state->skeletonState->rotationState[10];
    inverseRotationState[11] = state->skeletonState->rotationState[14];
    inverseRotationState[12] = state->skeletonState->rotationState[3];
    inverseRotationState[13] = state->skeletonState->rotationState[7];
    inverseRotationState[14] = state->skeletonState->rotationState[11];
    inverseRotationState[15] = state->skeletonState->rotationState[15];

    // calc matrix of one single rotation
    singleRotM[0] = rotCosT + pow(rotdy / rotdr, 2.) * (1. - rotCosT);
    singleRotM[1] = rotdx * rotdy / rotdr / rotdr * (rotCosT - 1.);
    singleRotM[2] = - rotdx / rotdr * rotSinT;
    singleRotM[3] = 0.;
    singleRotM[4] = singleRotM[1];
    singleRotM[5] = rotCosT + pow(rotdx / rotdr, 2.) * (1. - rotCosT);
    singleRotM[6] = - rotdy / rotdr * rotSinT;
    singleRotM[7] = 0.;
    singleRotM[8] = - singleRotM[2];
    singleRotM[9] = - singleRotM[6];
    singleRotM[10] = rotCosT;
    singleRotM[11] = 0.;
    singleRotM[12] = 0.;
    singleRotM[13] = 0.;
    singleRotM[14] = 0.;
    singleRotM[15] = 1.;

    // undo all previous rotations
    glMultMatrixf(inverseRotationState);

    // multiply all previous rotations to current rotation and overwrite state->skeletonState->rotationsState
    updateRotationStateMatrix(singleRotM,state->skeletonState->rotationState);

    //rotate to the new rotation state
    glMultMatrixf(state->skeletonState->rotationState);

    return true;
}


bool Renderer::setRotationState(uint32_t setTo) {
    if (setTo == 0){
            //Reset Viewport
            state->skeletonState->rotationState[0] = 0.866025;
            state->skeletonState->rotationState[1] = 0.286788;
            state->skeletonState->rotationState[2] = 0.409576;
            state->skeletonState->rotationState[3] = 0.0;
            state->skeletonState->rotationState[4] = -0.5;
            state->skeletonState->rotationState[5] = 0.496732;
            state->skeletonState->rotationState[6] = 0.709407;
            state->skeletonState->rotationState[7] = 0.0;
            state->skeletonState->rotationState[8] = 0.0;
            state->skeletonState->rotationState[9] = 0.819152;
            state->skeletonState->rotationState[10] = -0.573576;
            state->skeletonState->rotationState[11] = 0.0;
            state->skeletonState->rotationState[12] = 0.0;
            state->skeletonState->rotationState[13] = 0.0;
            state->skeletonState->rotationState[14] = 0.0;
            state->skeletonState->rotationState[15] = 1.0;
        }
        if (setTo == 1){
            //XY view
            state->skeletonState->rotationState[0] = 1.0;
            state->skeletonState->rotationState[1] = 0.0;
            state->skeletonState->rotationState[2] = 0.0;
            state->skeletonState->rotationState[3] = 0.0;
            state->skeletonState->rotationState[4] = 0.0;
            state->skeletonState->rotationState[5] = 1.0;
            state->skeletonState->rotationState[6] = 0.0;
            state->skeletonState->rotationState[7] = 0.0;
            state->skeletonState->rotationState[8] = 0.0;
            state->skeletonState->rotationState[9] = 0.0;
            state->skeletonState->rotationState[10] = 1.0;
            state->skeletonState->rotationState[11] = 0.0;
            state->skeletonState->rotationState[12] = 0.0;
            state->skeletonState->rotationState[13] = 0.0;
            state->skeletonState->rotationState[14] = 0.0;
            state->skeletonState->rotationState[15] = 1.0;
        }
        if (setTo == 2){
            //YZ view
            state->skeletonState->rotationState[0] = 1.0;
            state->skeletonState->rotationState[1] = 0.0;
            state->skeletonState->rotationState[2] = 0.0;
            state->skeletonState->rotationState[3] = 0.0;
            state->skeletonState->rotationState[4] = 0.0;
            state->skeletonState->rotationState[5] = 0.0;
            state->skeletonState->rotationState[6] = -1.0;
            state->skeletonState->rotationState[7] = 0.0;
            state->skeletonState->rotationState[8] = 0.0;
            state->skeletonState->rotationState[9] = 1.0;
            state->skeletonState->rotationState[10] = 0.0;
            state->skeletonState->rotationState[11] = 0.0;
            state->skeletonState->rotationState[12] = 0.0;
            state->skeletonState->rotationState[13] = 0.0;
            state->skeletonState->rotationState[14] = 0.0;
            state->skeletonState->rotationState[15] = 1.0;
        }
        if (setTo == 3){
            //XZ view
            state->skeletonState->rotationState[0] = 0.0;
            state->skeletonState->rotationState[1] = 0.0;
            state->skeletonState->rotationState[2] = -1.0;
            state->skeletonState->rotationState[3] = 0.0;
            state->skeletonState->rotationState[4] = 0.0;
            state->skeletonState->rotationState[5] = -1.0;
            state->skeletonState->rotationState[6] = 0.0;
            state->skeletonState->rotationState[7] = 0.0;
            state->skeletonState->rotationState[8] = -1.0;
            state->skeletonState->rotationState[9] = 0.0;
            state->skeletonState->rotationState[10] = 0.0;
            state->skeletonState->rotationState[11] = 0.0;
            state->skeletonState->rotationState[12] = 0.0;
            state->skeletonState->rotationState[13] = 0.0;
            state->skeletonState->rotationState[14] = 0.0;
            state->skeletonState->rotationState[15] = 1.0;
        }
    return true;
}
