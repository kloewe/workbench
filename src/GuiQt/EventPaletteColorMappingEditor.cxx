/*LICENSE_START*/ 
/* 
 *  Copyright 1995-2002 Washington University School of Medicine 
 * 
 *  http://brainmap.wustl.edu 
 * 
 *  This file is part of CARET. 
 * 
 *  CARET is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version. 
 * 
 *  CARET is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with CARET; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 * 
 */ 

#include "EventPaletteColorMappingEditor.h"

using namespace caret;

/**
 * Constructor.
 * @param browserWindowIndex
 *    Index of browser window.
 * @param mapFile
 *    Caret Mappable Data File.
 * @param mapIndex
 *    Map index in mapFile.
 */
EventPaletteColorMappingEditor::EventPaletteColorMappingEditor(const int32_t browserWindowIndex,
                                                               CaretMappableDataFile* mapFile,
                                                               const int32_t mapIndex)
: Event(EventTypeEnum::EVENT_PALETTE_COLOR_MAPPING_EDITOR)
{
    this->browserWindowIndex = browserWindowIndex;
    this->mapFile = mapFile;
    this->mapIndex = mapIndex;
}

/*
 * Destructor.
 */
EventPaletteColorMappingEditor::~EventPaletteColorMappingEditor()
{
    
}

