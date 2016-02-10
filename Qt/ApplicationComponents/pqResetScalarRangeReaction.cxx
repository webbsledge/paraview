/*=========================================================================

   Program: ParaView
   Module:    pqResetScalarRangeReaction.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pqResetScalarRangeReaction.h"

#include "pqActiveObjects.h"
#include "pqPipelineRepresentation.h"
#include "pqUndoStack.h"

#include <QDebug>

//-----------------------------------------------------------------------------
pqResetScalarRangeReaction::pqResetScalarRangeReaction(
  QAction* parentObject, bool track_active_objects)
  : Superclass(parentObject)
{
  if (track_active_objects)
    {
    QObject::connect(&pqActiveObjects::instance(),
      SIGNAL(representationChanged(pqDataRepresentation*)),
      this, SLOT(setRepresentation(pqDataRepresentation*)));
    this->setRepresentation(pqActiveObjects::instance().activeRepresentation());
    }
}

//-----------------------------------------------------------------------------
void pqResetScalarRangeReaction::setRepresentation(pqDataRepresentation* repr)
{
  this->Representation = qobject_cast<pqPipelineRepresentation*>(repr);
  this->updateEnableState();
}

//-----------------------------------------------------------------------------
void pqResetScalarRangeReaction::updateEnableState()
{
  this->parentAction()->setEnabled(this->Representation != NULL);
}

//-----------------------------------------------------------------------------
void pqResetScalarRangeReaction::onTriggered()
{
  pqResetScalarRangeReaction::resetScalarRange(this->Representation);
}

//-----------------------------------------------------------------------------
void pqResetScalarRangeReaction::resetScalarRange(pqPipelineRepresentation* repr)
{
  if (repr == NULL)
    {
    repr = qobject_cast<pqPipelineRepresentation*>(
      pqActiveObjects::instance().activeRepresentation());
    if (!repr)
      {
      qCritical() << "No representation provided.";
      return;
      }
    }

  BEGIN_UNDO_SET("Reset Range");
  repr->resetLookupTableScalarRange();
  repr->renderViewEventually();
  END_UNDO_SET();
}
