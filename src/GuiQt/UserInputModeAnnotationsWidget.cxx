
/*LICENSE_START*/
/*
 *  Copyright (C) 2015 Washington University School of Medicine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*LICENSE_END*/

#define __USER_INPUT_MODE_ANNOTATIONS_WIDGET_DECLARE__
#include "UserInputModeAnnotationsWidget.h"
#undef __USER_INPUT_MODE_ANNOTATIONS_WIDGET_DECLARE__

#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

#include "AnnotationCoordinateWidget.h"
#include "AnnotationColorWidget.h"
#include "AnnotationFontWidget.h"
#include "AnnotationFormatWidget.h"
#include "AnnotationInsertNewWidget.h"
#include "AnnotationLine.h"
#include "AnnotationLineArrowTipsWidget.h"
#include "AnnotationManager.h"
#include "AnnotationOneDimensionalShape.h"
#include "AnnotationRotationWidget.h"
#include "AnnotationText.h"
#include "AnnotationTextAlignmentWidget.h"
#include "AnnotationTextEditorWidget.h"
#include "AnnotationTextOrientationWidget.h"
#include "AnnotationWidthHeightWidget.h"
#include "Brain.h"
#include "CaretAssert.h"
#include "EventAnnotation.h"
#include "EventBrainReset.h"
#include "EventManager.h"
#include "EventUserInterfaceUpdate.h"
#include "GuiManager.h"
#include "UserInputModeAnnotations.h"
#include "WuQtUtilities.h"

using namespace caret;

/**
 * \class caret::UserInputModeAnnotationsWidget 
 * \brief Toolbar widget for annotations.
 * \ingroup GuiQt
 */

/**
 * Constructor.
 *
 * @param inputModeAnnotations
 *    Annotation mode input processor.
 * @param browserWindowIndex
 *    Index of browser window using this widget.
 */
UserInputModeAnnotationsWidget::UserInputModeAnnotationsWidget(UserInputModeAnnotations* inputModeAnnotations,
                                                               const int32_t browserWindowIndex)
: QWidget(),
m_browserWindowIndex(browserWindowIndex),
m_inputModeAnnotations(inputModeAnnotations)
{
    CaretAssert(inputModeAnnotations);
    
    m_textEditorWidget           = new AnnotationTextEditorWidget(m_browserWindowIndex);
    
    m_lineArrowTipsWidget        = new AnnotationLineArrowTipsWidget(m_browserWindowIndex);
    
    m_fontWidget                 = new AnnotationFontWidget(m_browserWindowIndex);
    
    m_colorWidget                = new AnnotationColorWidget(m_browserWindowIndex);
    
    m_textAlignmentWidget        = new AnnotationTextAlignmentWidget(m_browserWindowIndex);
    
    m_textOrientationWidget      = new AnnotationTextOrientationWidget(m_browserWindowIndex);
    
    m_coordinateOneWidget        = new AnnotationCoordinateWidget(m_browserWindowIndex);
    
    m_coordinateTwoWidget        = new AnnotationCoordinateWidget(m_browserWindowIndex);
    
    m_widthHeightWidget          = new AnnotationWidthHeightWidget(m_browserWindowIndex);
    
    m_rotationWidget             = new AnnotationRotationWidget(m_browserWindowIndex);
    
    m_formatWidget               = new AnnotationFormatWidget(m_browserWindowIndex);
    
    m_insertDeleteWidget         = new AnnotationInsertNewWidget(m_browserWindowIndex);
    
    /*
     * Connect signals for setting a coordinate with the mouse.
     */
    QObject::connect(m_coordinateOneWidget, SIGNAL(signalSelectCoordinateWithMouse()),
                     this, SLOT(selectCoordinateOneWithMouse()));
    QObject::connect(m_coordinateTwoWidget, SIGNAL(signalSelectCoordinateWithMouse()),
                     this, SLOT(selectCoordinateTwoWithMouse()));
    
    /*
     * Layout top row of widgets
     */
    QHBoxLayout* topRowLayout = new QHBoxLayout();
    WuQtUtilities::setLayoutSpacingAndMargins(topRowLayout, 2, 2);
    topRowLayout->addWidget(m_colorWidget, 0, Qt::AlignTop);
    topRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    topRowLayout->addWidget(m_lineArrowTipsWidget, 0, Qt::AlignTop);
    topRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    topRowLayout->addWidget(m_textEditorWidget, 0, Qt::AlignTop);
    topRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    topRowLayout->addWidget(m_fontWidget, 0, Qt::AlignTop);
    topRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    topRowLayout->addWidget(m_textAlignmentWidget, 0, Qt::AlignTop);
    topRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    topRowLayout->addWidget(m_textOrientationWidget, 0, Qt::AlignTop);
    topRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    topRowLayout->addWidget(m_insertDeleteWidget, 0, Qt::AlignTop);
    topRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    topRowLayout->addWidget(m_formatWidget, 0, Qt::AlignTop);
    topRowLayout->addStretch();
    
    /*
     * Layout bottom row of widgets
     */
    QHBoxLayout* bottomRowLayout = new QHBoxLayout();
    WuQtUtilities::setLayoutSpacingAndMargins(bottomRowLayout, 2, 2);
    bottomRowLayout->addWidget(m_coordinateOneWidget);
    bottomRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    bottomRowLayout->addWidget(m_coordinateTwoWidget);
    bottomRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    bottomRowLayout->addWidget(m_widthHeightWidget);
    bottomRowLayout->addWidget(WuQtUtilities::createVerticalLineWidget());
    bottomRowLayout->addWidget(m_rotationWidget);
    bottomRowLayout->addStretch();
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    WuQtUtilities::setLayoutSpacingAndMargins(layout, 0, 2);
    layout->addLayout(topRowLayout);
    layout->addWidget(WuQtUtilities::createHorizontalLineWidget());
    layout->addLayout(bottomRowLayout);
    
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_BRAIN_RESET);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_ANNOTATION);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_USER_INTERFACE_UPDATE);
}

/**
 * Destructor.
 */
UserInputModeAnnotationsWidget::~UserInputModeAnnotationsWidget()
{
    EventManager::get()->removeAllEventsFromListener(this);
}

/**
 * Receive an event.
 *
 * @param event
 *     The event that the receive can respond to.
 */
void
UserInputModeAnnotationsWidget::receiveEvent(Event* event)
{
    bool updateAnnotationWidgetsFlag = false;
    
    if (event->getEventType() == EventTypeEnum::EVENT_BRAIN_RESET) {
        EventBrainReset* brainEvent = dynamic_cast<EventBrainReset*>(event);
        CaretAssert(brainEvent);
        
        updateAnnotationWidgetsFlag = true;
        
        brainEvent->setEventProcessed();
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_ANNOTATION) {
        EventAnnotation* annotationEvent = dynamic_cast<EventAnnotation*>(event);
        CaretAssert(annotationEvent);
        updateAnnotationWidgetsFlag = true;
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_USER_INTERFACE_UPDATE) {
        EventUserInterfaceUpdate* updateEvent = dynamic_cast<EventUserInterfaceUpdate*>(event);
        CaretAssert(updateEvent);
        
        updateAnnotationWidgetsFlag = true;
        
    }
    else {
        return;
    }

    if (! updateAnnotationWidgetsFlag) {
        return;
    }
    
    updateWidget();
    
}

/**
 * Select coordinate one with the mouse.
 */
void
UserInputModeAnnotationsWidget::selectCoordinateOneWithMouse()
{
    m_inputModeAnnotations->setMode(UserInputModeAnnotations::MODE_SET_COORDINATE_ONE);
}

/**
 * Select coordinate two with the mouse.
 */
void
UserInputModeAnnotationsWidget::selectCoordinateTwoWithMouse()
{
    m_inputModeAnnotations->setMode(UserInputModeAnnotations::MODE_SET_COORDINATE_TWO);
}


/**
 * Update the widget.
 */
void
UserInputModeAnnotationsWidget::updateWidget()
{
    /*
     * Show the proper widget
     */
    switch (m_inputModeAnnotations->getMode()) {
        case UserInputModeAnnotations::MODE_NEW:
            break;
        case UserInputModeAnnotations::MODE_SELECT:
            break;
        case UserInputModeAnnotations::MODE_SET_COORDINATE_ONE:
            break;
        case UserInputModeAnnotations::MODE_SET_COORDINATE_TWO:
            break;
    }
    
    AnnotationManager* annotationManager = GuiManager::get()->getBrain()->getAnnotationManager();
    
    
    std::vector<Annotation*> selectedAnnotations = annotationManager->getSelectedAnnotations();
    
    Annotation* annotationBeingEdited = NULL;
    if (selectedAnnotations.size() == 1) {
        annotationBeingEdited = selectedAnnotations[0];
    }
    
    AnnotationLine* lineAnnotation = NULL;
    AnnotationText* textAnnotation = NULL;
    AnnotationTwoDimensionalShape* twoDimAnnotation = NULL;
    AnnotationOneDimensionalShape* oneDimAnnotation = NULL;
    
    AnnotationCoordinate* coordinateOne = NULL;
    AnnotationCoordinate* coordinateTwo = NULL;
    if (annotationBeingEdited != NULL) {
        textAnnotation   = dynamic_cast<AnnotationText*>(annotationBeingEdited);
        
        oneDimAnnotation = dynamic_cast<AnnotationOneDimensionalShape*>(annotationBeingEdited);
        if (oneDimAnnotation != NULL) {
            coordinateOne = oneDimAnnotation->getStartCoordinate();
            coordinateTwo = oneDimAnnotation->getEndCoordinate();
        }
        
        twoDimAnnotation = dynamic_cast<AnnotationTwoDimensionalShape*>(annotationBeingEdited);
        if (twoDimAnnotation != NULL) {
            coordinateOne = twoDimAnnotation->getCoordinate();
        }
        
        lineAnnotation = dynamic_cast<AnnotationLine*>(annotationBeingEdited);
    }
    
    m_fontWidget->updateContent(textAnnotation);
    m_textEditorWidget->updateContent(textAnnotation);
    m_colorWidget->updateContent(annotationBeingEdited);
    m_lineArrowTipsWidget->updateContent(lineAnnotation);
    m_textAlignmentWidget->updateContent(textAnnotation);
    m_textOrientationWidget->updateContent(textAnnotation);
    m_widthHeightWidget->updateContent(twoDimAnnotation);
    m_rotationWidget->updateContent(twoDimAnnotation);
    m_insertDeleteWidget->updateContent(annotationBeingEdited);
    
    AnnotationCoordinateSpaceEnum::Enum coordinateSpace = AnnotationCoordinateSpaceEnum::TAB;
    if (annotationBeingEdited != NULL) {
        coordinateSpace = annotationBeingEdited->getCoordinateSpace();
    }
    
    m_coordinateOneWidget->updateContent(coordinateSpace,
                                         coordinateOne);
    if (coordinateTwo != NULL) {
        m_coordinateTwoWidget->updateContent(coordinateSpace,
                                             coordinateTwo);
        m_coordinateTwoWidget->setVisible(true);
    }
    else {
        m_coordinateTwoWidget->setVisible(false);
    }
}


