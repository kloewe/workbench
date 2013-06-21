#ifndef __VOXEL_IJK_H__
#define __VOXEL_IJK_H__

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

#include "stdint.h"

namespace caret {
    
    struct VoxelIJK
    {
        int64_t m_ijk[3];//so it can be put in a vector
        VoxelIJK() { }
        VoxelIJK(int64_t i, int64_t j, int64_t k) { m_ijk[0] = i; m_ijk[1] = j; m_ijk[2] = k; }

        VoxelIJK(const int64_t ijk[3]) { 
            m_ijk[0] = ijk[0]; 
            m_ijk[1] = ijk[1]; 
            m_ijk[2] = ijk[2]; 
        }
    };
    
}

#endif //__VOXEL_IJK_H__
