/*=========================================================================

   Program: ParaView
   Module:  pqAnimationTimeWidget.h

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
#ifndef pqAnimationTimeWidget_h
#define pqAnimationTimeWidget_h

#include "pqComponentsModule.h"
#include "pqDoubleLineEdit.h" // for pqDoubleLineEdit::RealNumberNotation.

#include "vtkParaViewDeprecation.h"

#include <QList>
#include <QScopedPointer>
#include <QVariant>
#include <QWidget>
#include <vector>

class pqAnimationScene;
class vtkSMProxy;

/**
 * pqAnimationTimeWidget is a widget that can be used to show/set the current
 * animation time.
 * The widget allow the user to do the following:
 * \li View and/or change the current time value (in seq modes), or
 *     current time step value (in snap-to-timesteps mode).
 */
class PQCOMPONENTS_EXPORT pqAnimationTimeWidget : public QWidget
{
  Q_OBJECT
  Q_ENUMS(RealNumberNotation)
  Q_PROPERTY(QString playMode READ playMode WRITE setPlayMode NOTIFY playModeChanged)
  Q_PROPERTY(int numberOfFrames READ numberOfFrames WRITE setNumberOfFrames)
  Q_PROPERTY(double startTime READ startTime WRITE setStartTime)
  Q_PROPERTY(double endTime READ endTime WRITE setEndTime)
  Q_PROPERTY(QString timeLabel READ timeLabel WRITE setTimeLabel)
  Q_PROPERTY(QList<QVariant> timestepValues READ timestepValues WRITE setTimestepValues)

  typedef QWidget Superclass;

public:
  pqAnimationTimeWidget(QWidget* parent = nullptr);
  ~pqAnimationTimeWidget() override;
  using RealNumberNotation = pqDoubleLineEdit::RealNumberNotation;

  /**
   * Provides access to the animation scene proxy currently
   * controlled/reflected by this widget.
   */
  vtkSMProxy* animationScene() const;

  /**
   * Set the animation scene which is reflected/controlled by this widget.
   */
  void setAnimationScene(pqAnimationScene* animationScene);

  ///@{
  /**
   * Get/Set the list of timestep. `QList<QVariant>` is a list of variants
   * convertible to double.
   */
  void setTimestepValues(const QList<QVariant>& list);
  const QList<QVariant>& timestepValues() const;
  ///@}

  ///@{
  /**
   * Get/Set the animation start time.
   */
  void setStartTime(double start);
  double startTime() const;
  ///@}

  ///@{
  /**
   * Get/Set the animation end time.
   */
  void setEndTime(double end);
  double endTime() const;
  ///@}

  ///@{
  /**
   * Get/Set the number of frames.
   */
  void setNumberOfFrames(int nbOfFrame);
  int numberOfFrames() const;
  ///@}

  /**
   * Set the current animation time
   */
  void setCurrentTime(double t);

  /**
   * Return the notation used to display the number.
   * \sa setNotation()
   */
  RealNumberNotation notation() const;

  /**
   * Return the precision used to display the number.
   * \sa setPrecision()
   */
  int precision() const;

  ///@{
  /**
   * Get/set the animation playback mode.
   */
  void setPlayMode(const QString& mode);
  QString playMode() const;
  ///@}

  ///@{
  /**
   * Get/set the label text to use for the "time" parameter.
   */
  void setTimeLabel(const QString& val);
  QString timeLabel() const;
  ///@}

Q_SIGNALS:
  void playModeChanged();
  void dummySignal();

public Q_SLOTS:
  /**
   * Set the notation used to display the number.
   * \sa notation()
   */
  PARAVIEW_DEPRECATED_IN_5_12_0("Unused. Display rely on global setting")
  void setNotation(RealNumberNotation _notation);

  /**
   * Set the precision used to display the number.
   * \sa precision()
   */
  PARAVIEW_DEPRECATED_IN_5_12_0("Unused. Display rely on global setting")
  void setPrecision(int precision);

  /**
   * Re render the widget.
   * This can be helpful when notation/precision settings have changed.
   */
  void render();

protected Q_SLOTS:
  /**
   * Update the current time in the widget GUI
   */
  void updateCurrentTime(double t);

private:
  Q_DISABLE_COPY(pqAnimationTimeWidget)

  class pqInternals;
  QScopedPointer<pqInternals> Internals;
};

#endif
