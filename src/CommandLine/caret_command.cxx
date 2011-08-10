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

#include <iostream>

#include "CaretAssert.h"
#include "CommandOperationManager.h"
#include "ProgramParameters.h"
#include "SystemUtilities.h"

using namespace caret;


static void runCommand(int argc, char* argv[]) {
    
    ProgramParameters parameters(argc, argv);
    
    CommandOperationManager* commandManager = NULL;
    try {
        commandManager = CommandOperationManager::getCommandOperationManager();
        
        commandManager->runCommand(parameters);
        
    }
    catch (CommandException& e) {
        std::cout << "ERROR, Command Failed: " << e.whatString().toStdString() << std::endl;
    }
    
    if (commandManager != NULL) {
        CommandOperationManager::deleteCommandOperationManager();
    }
}

int main(int argc, char* argv[]) {

    SystemUtilities::setHandlersForUnexpected();
    
    runCommand(argc, argv);
    
    /*
     * See if any objects were not deleted.
     */
    CaretObject::printListOfObjectsNotDeleted(true);
}
  
