#ifndef __SELECTION_ITEM_ANNOTATION_H__
#define __SELECTION_ITEM_ANNOTATION_H__

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

#include "AnnotationSizingHandleTypeEnum.h"
#include "SelectionItem.h"



namespace caret {
    class Annotation;
    class AnnotationFile;
    
    class SelectionItemAnnotation : public SelectionItem {
        
    public:
        SelectionItemAnnotation();
        
        virtual ~SelectionItemAnnotation();
        
        virtual bool isValid() const;
        
        void reset();
        
        virtual AString toString() const;
        
        Annotation* getAnnotation() const;
        
        AnnotationFile* getAnnotationFile() const;
        
        AnnotationSizingHandleTypeEnum::Enum getSizingHandle() const;
        
        void setAnnotation(AnnotationFile* annotationFile,
                           Annotation* annotation,
                           const AnnotationSizingHandleTypeEnum::Enum annotationSizingHandle);

        // ADD_NEW_METHODS_HERE

    private:
        SelectionItemAnnotation(const SelectionItemAnnotation&);

        SelectionItemAnnotation& operator=(const SelectionItemAnnotation&);
        
        AnnotationFile* m_annotationFile;
        
        Annotation* m_annotation;
        
        AnnotationSizingHandleTypeEnum::Enum m_sizingHandle;
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __SELECTION_ITEM_ANNOTATION_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __SELECTION_ITEM_ANNOTATION_DECLARE__

} // namespace
#endif  //__SELECTION_ITEM_ANNOTATION_H__
