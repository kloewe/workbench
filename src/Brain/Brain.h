#ifndef __BRAIN_H__
#define __BRAIN_H__

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
/*LICENSE_END*/

#include <vector>
#include <stdint.h>

#include "CaretObject.h"
#include "DataFileTypeEnum.h"
#include "DataFileException.h"
#include "EventListenerInterface.h"
#include "StructureEnum.h"

namespace caret {
    
    class BrainStructure;
    class EventDataFileRead;
    class EventSpecFileReadDataFiles;
    class ModelDisplayControllerWholeBrain;
    class PaletteFile;
    class SpecFile;
    class VolumeFile;
    
    class Brain : public CaretObject, public EventListenerInterface {

    public:
        Brain();
        
        ~Brain();
        
    private:
        Brain(const Brain&);
        Brain& operator=(const Brain&);
        
    public:
        int getNumberOfBrainStructures() const;
        
        void addBrainStructure(BrainStructure* brainStructure);
        
        BrainStructure* getBrainStructure(const int32_t indx);

        BrainStructure* getBrainStructure(StructureEnum::Enum structure,
                                          bool createIfNotFound);

        PaletteFile* getPaletteFile();
        
        SpecFile* getSpecFile();
        
        void loadFilesSelectedInSpecFile(EventSpecFileReadDataFiles* readSpecFileDataFilesEvent);
        
        void resetBrain();
        
        void receiveEvent(Event* event);
        
    private:
        void processReadDataFileEvent(EventDataFileRead* readDataFileEvent);
        
        void readDataFile(const DataFileTypeEnum::Enum dataFileType,
                          const StructureEnum::Enum structure,
                          const AString& dataFileName) throw (DataFileException);
        
        void readLabelFile(const AString& filename,
                           const StructureEnum::Enum structure) throw (DataFileException);
        
        void readMetricFile(const AString& filename,
                            const StructureEnum::Enum structure) throw (DataFileException);
        
        void readRgbaFile(const AString& filename,
                          const StructureEnum::Enum structure) throw (DataFileException);
        
        void readSurfaceFile(const AString& filename,
                             const StructureEnum::Enum structure) throw (DataFileException);
        
        void readVolumeFile(const AString& filename) throw (DataFileException);
                            
        void readBorderProjectionFile(const AString& filename) throw (DataFileException);
        
        void readConnectivityFile(const AString& filename) throw (DataFileException);
        
        void readFociProjectionFile(const AString& filename) throw (DataFileException);
        
        void readPaletteFile(const AString& filename) throw (DataFileException);
        
        void readSceneFile(const AString& filename) throw (DataFileException);
        
        AString updateFileNameForReading(const AString& filename);
        
        void updateWholeBrainController();
        
        std::vector<BrainStructure*> brainStructures;
        
        PaletteFile* paletteFile;
        
        AString currentDirectory;
        
        SpecFile* specFile;
        
        std::vector<VolumeFile*> volumeFiles;
        
        ModelDisplayControllerWholeBrain* wholeBrainController;
        
    };

} // namespace

#endif // __BRAIN_H__

