/*
 *  This file is a part of KNOSSOS.
 *
 *  (C) Copyright 2007-2013
 *  Max-Planck-Gesellschaft zur Foerderung der Wissenschaften e.V.
 *
 *  KNOSSOS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 of
 *  the License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * For further information, visit http://www.knossostool.org or contact
 *     Joergen.Kornfeld@mpimf-heidelberg.mpg.de or
 *     Fabian.Svara@mpimf-heidelberg.mpg.de
 */
#include "knossos-global.h"
#include "remote.h"
#include "functions.h"
#include <QDebug>
#include <math.h>

extern stateInfo *state;

Remote::Remote(QObject *parent) : QThread(parent) {}

void Remote::run() {
    floatCoordinate currToNext; //distance vector
    // remoteSignal is != false as long as the remote is active.
    // Checking for remoteSignal is therefore a way of seeing if the remote
    // is available for doing something.
    //
    // Depending on the contents of remoteState, this thread will either go
    // on to listen to a socket and get its instructions from there or it
    // will follow the trajectory given in a file.

    while(true) {
        state->protectRemoteSignal->lock();
        while(!state->remoteSignal) {
            state->conditionRemoteSignal->wait(state->protectRemoteSignal);
        }

        state->remoteSignal = false;
        state->protectRemoteSignal->unlock();

        if(state->quitSignal) {
            break;
        }

        SET_COORDINATE (currToNext, state->viewerState->currentPosition.x - this->recenteringPosition.x,
        state->viewerState->currentPosition.y - this->recenteringPosition.y,
        state->viewerState->currentPosition.z - this->recenteringPosition.z);
        if(euclidicNorm(&currToNext) > JMP_THRESHOLD) {
            remoteJump(this->recenteringPosition.x,
                       this->recenteringPosition.y,
                       this->recenteringPosition.z);
        } else {
            remoteWalk(this->recenteringPosition.x - state->viewerState->currentPosition.x,
                       this->recenteringPosition.y - state->viewerState->currentPosition.y,
                       this->recenteringPosition.z - state->viewerState->currentPosition.z);
        }

        if(state->quitSignal == true) {
            break;
        }
    }
}

/**
 * @attention jump event is replaced with userMoveSignal
 */
bool Remote::remoteJump(int x, int y, int z) {
    // is not threadsafe

    emit userMoveSignal(x - state->viewerState->currentPosition.x,
                        y - state->viewerState->currentPosition.y,
                        z - state->viewerState->currentPosition.z);

    return true;
}

void Remote::msleep(unsigned long msec) {
    QThread::msleep(msec);
}

/**
 * @attention moveEvent is replaced  by userMoveSignal
 */
bool Remote::remoteWalk(int x, int y, int z) {

    /*
    * This function breaks the big walk distance into many small movements
    * where the maximum length of the movement along any single axis is
    * equal to the magnification, i.e. in mag4 it is 4.
    * As we cannot move by fractions, this function keeps track of
    * residuals that add up to make a movement of an integer along an
    * axis every once in a while.
    * An alternative would be to store the currentPosition as a float or
    * double but that has its own problems. We might do it in the future,
    * though.
    * Possible improvement to this function: Make the length of a complete
    * singleMove to match mag, not the length of the movement on one axis.
    *
    */

    /*
    * BUG: For some reason, events to the viewer seem to become lost under
    * some circumstances, resulting in incorrect paths when calling
    * remoteWalk(). The remoteWalkTo() wrapper takes care of that, but the
    * problem should be addressed in a more general way - I just don't kow
    * how, yet.
    *
    */

    floatCoordinate singleMove;
    floatCoordinate residuals;
    Coordinate doMove;
    int totalMoves = 0, i = 0;
    int eventDelay = 0;
    floatCoordinate walkVector;
    float walkLength = 0.;
    uint timePerStep = 0;
    uint recenteringTime = 0;

    //float tempAlphaCache =  state->viewerState->alphaCache;
    float tempAlpha = state->alpha;
    //float tempBetaCache =  state->viewerState->betaCache;
    float tempBeta = state->beta;

    float alpha, beta, dAlpha, dBeta;
    int dx = this->recenteringPosition.x - state->viewerState->lastRecenteringPosition.x;
    int dy = this->recenteringPosition.y -state->viewerState->lastRecenteringPosition.y;
    int dz = this->recenteringPosition.z -state->viewerState->lastRecenteringPosition.z;
    state->viewerState->lastRecenteringPosition.x = this->recenteringPosition.x;
    state->viewerState->lastRecenteringPosition.y = this->recenteringPosition.y;
    state->viewerState->lastRecenteringPosition.z = this->recenteringPosition.z;
    walkVector.x = (float) x;
    walkVector.y = (float) y;
    walkVector.z = (float) z;

    //Not locked...

    if (state->viewerState->recenteringTime > 5000){
        state->viewerState->recenteringTime = 5000;
        emit updateViewerStateSignal();

    }
    if (state->viewerState->recenteringTimeOrth < 10){
        state->viewerState->recenteringTimeOrth = 10;
        emit updateViewerStateSignal();

    }
    if (state->viewerState->recenteringTimeOrth > 5000){
        state->viewerState->recenteringTimeOrth = 5000;
        emit updateViewerStateSignal();
    }

    if (state->viewerState->walkOrth == false){
        recenteringTime = state->viewerState->recenteringTime;
    }
    else {
        recenteringTime = state->viewerState->recenteringTimeOrth;
        state->viewerState->walkOrth = false;
    }
    if ((state->viewerState->autoTracingMode != 0) && (state->viewerState->walkOrth == false)){
        recenteringTime = state->viewerState->autoTracingSteps * state->viewerState->autoTracingDelay;
    }

    walkLength = euclidicNorm(&walkVector);

    if(walkLength < 10.) walkLength = 10.;

    timePerStep = recenteringTime / ((uint)walkLength);

    if(timePerStep < 10) timePerStep = 10;

    //emit userMoveSignal(walkVector.x, walkVector.y, walkVector.z, TELL_COORDINATE_CHANGE);

    if(state->viewerState->stepsPerSec > 0)
        eventDelay = 1000 / state->viewerState->stepsPerSec;
    else
        eventDelay = 50;

    eventDelay = timePerStep;

    if(abs(x) >= abs(y) && abs(x) >= abs(z)) {
        totalMoves = abs(x) / state->magnification;
    }
    if(abs(y) >= abs(x) && abs(y) >= abs(z)) {
        totalMoves = abs(y) / state->magnification;

    }
    if(abs(z) >= abs(x) && abs(z) >= abs(y)) {
        totalMoves = abs(z) / state->magnification;
    }

    if(dx == 0 && dy == 0) {
        dAlpha = 0;
        if(dz == 0) {
            dBeta = 0;
        }
    }

    else{
        if(dy <= 0) {
            alpha = 360 - acos((double)dx/
                               sqrtf(powf((float)dx, 2.)
                                     + powf((float)dy, 2.)
                               )
                          )
                        /PI * 180;
        }
        else {
            alpha = acos((double)dx/
                         sqrtf(powf((float)dx, 2.)
                               + powf((float)dy, 2.))
                         )
                    /PI * 180;
        }

        beta = 180 + acos((double)dz/
                          sqrtf(powf((float)dx, 2.)
                                + powf((float)dy, 2.)
                                + powf((float)dz, 2.))
                          )
                /PI *180;

        if(abs(alpha - tempAlpha) <= 180) {
            dAlpha = alpha - tempAlpha;
        }
        else if(tempAlpha > alpha) {
            dAlpha = alpha +  360 - tempAlpha;
        }
        else if(tempAlpha == alpha) {
            dAlpha = 0;
        }
        else {
            dAlpha = alpha - 360 - tempAlpha;
        }

        if(abs(beta - tempBeta) <= 180) {
            dBeta = beta - tempBeta;
        }
        else if(tempBeta > beta) {
            dBeta = beta +  360 - tempBeta;
        }
        else if(tempBeta == beta) {
            dBeta = 0;
        }
        else {
            dBeta = beta - 360 - tempBeta;
        }
    }

    singleMove.x = (float)x / (float)totalMoves;
    singleMove.y = (float)y / (float)totalMoves;
    singleMove.z = (float)z / (float)totalMoves;
    dAlpha = dAlpha / (float)totalMoves;
    dBeta = dBeta / (float)totalMoves;

    SET_COORDINATE(residuals, 0, 0, 0);
    Coordinate sendMove; // next position to move to
    for(i = 0; i < totalMoves; i++) {
        SET_COORDINATE(doMove, 0, 0, 0);

        state->viewerState->alphaCache += dAlpha;
        state->viewerState->betaCache += dBeta;
        residuals.x += singleMove.x;
        residuals.y += singleMove.y;
        residuals.z += singleMove.z;

        if(residuals.x >= state->magnification) {
            doMove.x = state->magnification;
            residuals.x -= state->magnification;
        }
        else if(residuals.x <= -state->magnification) {
            doMove.x = -state->magnification;
            residuals.x += state->magnification;
        }

        if(residuals.y >= state->magnification) {
            doMove.y = state->magnification;
            residuals.y -= state->magnification;
        }
        else if(residuals.y <= -state->magnification) {
            doMove.y = -state->magnification;
            residuals.y += state->magnification;
        }

        if(residuals.z >= state->magnification) {
            doMove.z = state->magnification;
            residuals.z -= state->magnification;
        }
        else if(residuals.z <= -state->magnification) {
            doMove.z = -state->magnification;
            residuals.z += state->magnification;
        }

        if(doMove.x != 0 || doMove.z != 0 || doMove.y != 0) {
            sendMove.x = doMove.x;
            sendMove.y = doMove.y;
            sendMove.z = doMove.z;

            if(x == 0) {
                sendMove.x = 0;
            }
            if(y == 0) {
                sendMove.y = 0;
            }
            if(z == 0) {
                sendMove.z = 0;
            }
            emit userMoveSignal(sendMove.x, sendMove.y, sendMove.z);
        }
        // This is, of course, not really correct as the time of running
        // the loop body would need to be accounted for. But SDL_Delay()
        // granularity isn't fine enough and it doesn't matter anyway.
        msleep(eventDelay);
    }
    return true;
}

void Remote::setRecenteringPosition(int x, int y, int z) {
    this->recenteringPosition.x = x;
    this->recenteringPosition.y = y;
    this->recenteringPosition.z = z;
}

