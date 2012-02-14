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

#include "AlgorithmVolumeSmoothing.h"
#include "AlgorithmException.h"
#include "VolumeFile.h"
#include "Vector3D.h"
#include "CaretLogger.h"
#include "CaretOMP.h"
#include <cmath>

using namespace caret;
using namespace std;

AString AlgorithmVolumeSmoothing::getCommandSwitch()
{
    return "-volume-smoothing";
}

AString AlgorithmVolumeSmoothing::getShortDescription()
{
    return "SMOOTH A VOLUME FILE";
}

OperationParameters* AlgorithmVolumeSmoothing::getParameters()
{
    OperationParameters* ret = new OperationParameters();
    ret->addVolumeParameter(1, "volume-in", "the volume to smooth");
    
    ret->addDoubleParameter(2, "kernel", "the gaussian smoothing kernel sigma, in mm");
    
    ret->addVolumeOutputParameter(3, "volume-out", "the output volume");
    
    OptionalParameter* roiVolOpt = ret->createOptionalParameter(4, "-roi", "smooth only from data within an ROI");
    roiVolOpt->addVolumeParameter(1, "roivol", "the volume to use as an ROI");
    
    ret->createOptionalParameter(5, "-fix-zeros", "treat zero values as not being data");
    
    OptionalParameter* subvolSelect = ret->createOptionalParameter(6, "-subvolume", "select a single subvolume to smooth");
    subvolSelect->addStringParameter(1, "subvol", "the subvolume number or name");
    
    ret->setHelpText(
        AString("Gaussian smoothing for volumes.  By default, smooths all subvolumes with no ROI, if ROI is given, only ") +
        "positive voxels in the ROI volume have their values used, and all other voxels are set to zero.  Smoothing a non-orthogonal volume will " +
        "be significantly slower, because the operation cannot be separated into 1-dimensional smoothings without distorting the kernel shape.\n\n" +
        "The -fix-zeros option causes the smoothing to not use an input value if it is zero, but still write a smoothed value to the voxel.  " +
        "This is useful for zeros that indicate lack of information, preventing them from pulling down the intensity of nearby voxels, while " +
        "giving the zero an extrapolated value."
    );
    return ret;
}

void AlgorithmVolumeSmoothing::useParameters(OperationParameters* myParams, ProgressObject* myProgObj)
{
    VolumeFile* myVol = myParams->getVolume(1);
    float myKernel = (float)myParams->getDouble(2);
    VolumeFile* myOutVol = myParams->getOutputVolume(3);
    OptionalParameter* roiVolOpt = myParams->getOptionalParameter(4);
    VolumeFile* roiVol = NULL;
    if (roiVolOpt->m_present)
    {
        roiVol = roiVolOpt->getVolume(1);
    }
    OptionalParameter* fixZerosOpt = myParams->getOptionalParameter(5);
    bool fixZeros = fixZerosOpt->m_present;
    OptionalParameter* subvolSelect = myParams->getOptionalParameter(6);
    int subvolNum = -1;
    if (subvolSelect->m_present)
    {
        subvolNum = (int)myVol->getMapIndexFromNameOrNumber(subvolSelect->getString(1));
        if (subvolNum < 0)
        {
            throw AlgorithmException("invalid subvolume specified");
        }
    }
    AlgorithmVolumeSmoothing(myProgObj, myVol, myKernel, myOutVol, roiVol, fixZeros, subvolNum);
}

AlgorithmVolumeSmoothing::AlgorithmVolumeSmoothing(ProgressObject* myProgObj, const VolumeFile* inVol, const float& kernel, VolumeFile* outVol, const VolumeFile* roiVol, const bool& fixZeros, const int& subvol) : AbstractAlgorithm(myProgObj)
{
    LevelProgress myProgress(myProgObj);
    if (roiVol != NULL && !inVol->matchesVolumeSpace(roiVol))
    {
        throw AlgorithmException("volume roi space does not match input volume");
    }
    vector<int64_t> myDims;
    inVol->getDimensions(myDims);
    if (subvol < -1 || subvol >= myDims[3])
    {
        throw AlgorithmException("invalid subvolume specified");
    }
    CaretArray<float> scratchFrame(myDims[0] * myDims[1] * myDims[2]);//it could be faster to preinitialize with zeros, then generate a usable voxels list if there is a small ROI...
    float kernBox = kernel * 3.0f;
    vector<vector<float> > volSpace = inVol->getVolumeSpace();
    Vector3D ivec, jvec, kvec, origin, ijorth, jkorth, kiorth;
    ivec[0] = volSpace[0][0]; jvec[0] = volSpace[0][1]; kvec[0] = volSpace[0][2]; origin[0] = volSpace[0][3];
    ivec[1] = volSpace[1][0]; jvec[1] = volSpace[1][1]; kvec[1] = volSpace[1][2]; origin[1] = volSpace[1][3];
    ivec[2] = volSpace[2][0]; jvec[2] = volSpace[2][1]; kvec[2] = volSpace[2][2]; origin[2] = volSpace[2][3];
    const float ORTH_TOLERANCE = 0.001f;//tolerate this much deviation from orthogonal (dot product divided by product of lengths) to use orthogonal assumptions to smooth
    if (abs(ivec.dot(jvec.normal())) / ivec.length() < ORTH_TOLERANCE && abs(jvec.dot(kvec.normal())) / jvec.length() < ORTH_TOLERANCE && abs(kvec.dot(ivec.normal())) / kvec.length() < ORTH_TOLERANCE)
    {//if our axes are orthogonal, optimize by doing three 1-dimensional smoothings for O(voxels * (ki + kj + kk)) instead of O(voxels * (ki * kj * kk))
        CaretArray<float> scratchFrame2(myDims[0] * myDims[1] * myDims[2]);
        float ispace = ivec.length(), jspace = jvec.length(), kspace = kvec.length();
        int irange = (int)floor(kernBox / ispace);
        int jrange = (int)floor(kernBox / ispace);
        int krange = (int)floor(kernBox / ispace);
        if (irange < 1) irange = 1;//don't underflow
        if (jrange < 1) jrange = 1;
        if (krange < 1) krange = 1;
        int isize = irange * 2 + 1;//and construct a precomputed kernel in the box
        int jsize = jrange * 2 + 1;
        int ksize = krange * 2 + 1;
        CaretArray<float> iweights(isize), jweights(jsize), kweights(ksize);
        for (int i = 0; i < isize; ++i)
        {
            float tempf = ispace * (i - irange) / kernel;
            iweights[i] = exp(-tempf * tempf / 2.0f);
        }
        for (int j = 0; j < jsize; ++j)
        {
            float tempf = jspace * (j - jrange) / kernel;
            jweights[j] = exp(-tempf * tempf / 2.0f);
        }
        for (int k = 0; k < ksize; ++k)
        {
            float tempf = kspace * (k - krange) / kernel;
            kweights[k] = exp(-tempf * tempf / 2.0f);
        }
        if (subvol == -1)
        {
            vector<int64_t> origDims = inVol->getOriginalDimensions();
            outVol->reinitialize(origDims, volSpace, myDims[4]);
            for (int s = 0; s < myDims[3]; ++s)
            {
                outVol->setMapName(s, inVol->getMapName(s) + ", smooth " + AString::number(kernel));
                for (int c = 0; c < myDims[4]; ++c)
                {
                    const float* inFrame = inVol->getFrame(s, c);
                    smoothFrame(inFrame, myDims, scratchFrame, scratchFrame2, inVol, roiVol, iweights, jweights, kweights, irange, jrange, krange, fixZeros);
                    outVol->setFrame(scratchFrame, s, c);
                }
            }
        } else {
            vector<int64_t> origDims = inVol->getOriginalDimensions(), newDims;
            newDims.resize(3);
            newDims[0] = origDims[0];
            newDims[1] = origDims[1];
            newDims[2] = origDims[2];
            outVol->reinitialize(newDims, volSpace, myDims[4]);
            outVol->setMapName(0, inVol->getMapName(subvol) + ", smooth " + AString::number(kernel));
            for (int c = 0; c < myDims[4]; ++c)
            {
                const float* inFrame = inVol->getFrame(subvol, c);
                smoothFrame(inFrame, myDims, scratchFrame, scratchFrame2, inVol, roiVol, iweights, jweights, kweights, irange, jrange, krange, fixZeros);
                outVol->setFrame(scratchFrame, 0, c);
            }
        }
    } else {
        CaretLogWarning("input volume is not orthogonal, smoothing will take longer");
        ijorth = ivec.cross(jvec).normal();//find the bounding box that encloses a sphere of radius kernBox
        jkorth = jvec.cross(kvec).normal();
        kiorth = kvec.cross(ivec).normal();
        int irange = (int)floor(abs(kernBox / ivec.dot(jkorth)));
        int jrange = (int)floor(abs(kernBox / jvec.dot(kiorth)));
        int krange = (int)floor(abs(kernBox / kvec.dot(ijorth)));
        if (irange < 1) irange = 1;//don't underflow
        if (jrange < 1) jrange = 1;
        if (krange < 1) krange = 1;
        int isize = irange * 2 + 1;//and construct a precomputed kernel in the box
        int jsize = jrange * 2 + 1;
        int ksize = krange * 2 + 1;
        CaretArray<float**> weights(ksize);//so I don't need to explicitly delete[] if I throw
        CaretArray<float*> weights2(ksize * jsize);//construct flat arrays and index them into 3D
        CaretArray<float> weights3(ksize * jsize * isize);//index i comes last because that is linear for volume frames
        Vector3D kscratch, jscratch, iscratch;
        for (int k = 0; k < ksize; ++k)
        {
            kscratch = kvec * (k - krange);
            weights[k] = weights2 + k * jsize;
            for (int j = 0; j < jsize; ++j)
            {
                jscratch = kscratch + jvec * (j - jrange);
                weights[k][j] = weights3 + ((k * jsize) + j) * isize;
                for (int i = 0; i < isize; ++i)
                {
                    iscratch = jscratch + ivec * (i - irange);
                    float tempf = iscratch.length();
                    if (tempf > kernBox)
                    {
                        weights[k][j][i] = 0.0f;//test for zero to avoid some multiplies/adds, cheaper or cleaner than checking bounds on indexes from an index list
                    } else {
                        weights[k][j][i] = exp(-tempf * tempf / kernel / kernel / 2.0f);//optimization here isn't critical
                    }
                }
            }
        }
        if (subvol == -1)
        {
            vector<int64_t> origDims = inVol->getOriginalDimensions();
            outVol->reinitialize(origDims, volSpace, myDims[4]);
            for (int s = 0; s < myDims[3]; ++s)
            {
                outVol->setMapName(s, inVol->getMapName(s) + ", smooth " + AString::number(kernel));
                for (int c = 0; c < myDims[4]; ++c)
                {
                    const float* inFrame = inVol->getFrame(s, c);
                    smoothFrameNonOrth(inFrame, myDims, scratchFrame, inVol, roiVol, weights, irange, jrange, krange, fixZeros);
                    outVol->setFrame(scratchFrame, s, c);
                }
            }
        } else {
            vector<int64_t> origDims = inVol->getOriginalDimensions(), newDims;
            newDims.resize(3);
            newDims[0] = origDims[0];
            newDims[1] = origDims[1];
            newDims[2] = origDims[2];
            outVol->reinitialize(newDims, volSpace, myDims[4]);
            outVol->setMapName(0, inVol->getMapName(subvol) + ", smooth " + AString::number(kernel));
            for (int c = 0; c < myDims[4]; ++c)
            {
                const float* inFrame = inVol->getFrame(subvol, c);
                smoothFrameNonOrth(inFrame, myDims, scratchFrame, inVol, roiVol, weights, irange, jrange, krange, fixZeros);
                outVol->setFrame(scratchFrame, 0, c);
            }
        }
    }
}

void AlgorithmVolumeSmoothing::smoothFrame(const float* inFrame, vector<int64_t> myDims, CaretArray<float> scratchFrame, CaretArray<float> scratchFrame2, const VolumeFile* inVol, const VolumeFile* roiVol, CaretArray<float> iweights, CaretArray<float> jweights, CaretArray<float> kweights, int irange, int jrange, int krange, const bool& fixZeros)
{//this function should ONLY get invoked when the volume is orthogonal (axes are perpendicular, not necessarily aligned with x, y, z, and not necessarily equal spacing)
    const float* roiFrame = NULL;//it separates the 3-d smoothing into 3 successive 1-d smoothings to avoid looping over a box for each voxel (instead, loops over 3 lines)
    if (roiVol != NULL)//if this function ever needs more speed (unlikely), try having the j and k smoothings copy a line to thread private scratch so threads don't contend for scattered memory access
    {
        roiFrame = roiVol->getFrame();
    }
#pragma omp CARET_PARFOR schedule(dynamic)
    for (int k = 0; k < myDims[2]; ++k)//smooth along i axis
    {
        for (int j = 0; j < myDims[1]; ++j)
        {
            for (int i = 0; i < myDims[0]; ++i)
            {
                if (roiVol == NULL || roiVol->getValue(i, j, k) > 0.0f)
                {
                    int imin = i - irange, imax = i + irange + 1;//one-after array size convention
                    if (imin < 0) imin = 0;
                    if (imax > myDims[0]) imax = myDims[0];
                    float sum = 0.0f, weightsum = 0.0f;
                    int64_t baseInd = inVol->getIndex(0, j, k);
                    for (int ikern = imin; ikern < imax; ++ikern)
                    {
                        int64_t thisIndex = baseInd + ikern;
                        if ((roiVol == NULL || roiFrame[thisIndex] > 0.0f) && (!fixZeros || inFrame[thisIndex] != 0.0f))
                        {
                            float weight = iweights[ikern - i + irange];
                            weightsum += weight;
                            sum += weight * inFrame[thisIndex];
                        }
                    }
                    if (weightsum != 0.0f)
                    {
                        scratchFrame[inVol->getIndex(i, j, k)] = sum / weightsum;
                    } else {
                        scratchFrame[inVol->getIndex(i, j, k)] = 0.0f;
                    }
                } else {
                    scratchFrame[inVol->getIndex(i, j, k)] = 0.0f;
                }
            }
        }
    }
#pragma omp CARET_PARFOR schedule(dynamic)
    for (int k = 0; k < myDims[2]; ++k)//now j
    {
        for (int i = 0; i < myDims[0]; ++i)
        {
            for (int j = 0; j < myDims[1]; ++j)//step along the dimension being smoothed last for best cache coherence
            {
                if (roiVol == NULL || roiVol->getValue(i, j, k) > 0.0f)
                {
                    int jmin = j - jrange, jmax = j + jrange + 1;//one-after array size convention
                    if (jmin < 0) jmin = 0;
                    if (jmax > myDims[1]) jmax = myDims[1];
                    float sum = 0.0f, weightsum = 0.0f;
                    int64_t baseInd = inVol->getIndex(i, 0, k);
                    for (int jkern = jmin; jkern < jmax; ++jkern)
                    {
                        int64_t thisIndex = baseInd + jkern * myDims[0];
                        if ((roiVol == NULL || roiFrame[thisIndex] > 0.0f) && (!fixZeros || scratchFrame[thisIndex] != 0.0f))
                        {
                            float weight = jweights[jkern - j + jrange];
                            weightsum += weight;
                            sum += weight * scratchFrame[thisIndex];
                        }
                    }
                    if (weightsum != 0.0f)
                    {
                        scratchFrame2[inVol->getIndex(i, j, k)] = sum / weightsum;
                    } else {
                        scratchFrame2[inVol->getIndex(i, j, k)] = 0.0f;
                    }
                } else {
                    scratchFrame2[inVol->getIndex(i, j, k)] = 0.0f;
                }
            }
        }
    }
#pragma omp CARET_PARFOR schedule(dynamic)
    for (int j = 0; j < myDims[1]; ++j)//and finally k
    {
        for (int i = 0; i < myDims[0]; ++i)
        {
            for (int k = 0; k < myDims[2]; ++k)//ditto
            {
                if (roiVol == NULL || roiVol->getValue(i, j, k) > 0.0f)
                {
                    int kmin = k - krange, kmax = k + krange + 1;//one-after array size convention
                    if (kmin < 0) kmin = 0;
                    if (kmax > myDims[2]) kmax = myDims[2];
                    float sum = 0.0f, weightsum = 0.0f;
                    int64_t baseInd = inVol->getIndex(i, j, 0);
                    for (int kkern = kmin; kkern < kmax; ++kkern)
                    {
                        int64_t thisIndex = baseInd + kkern * myDims[0] * myDims[1];
                        if ((roiVol == NULL || roiFrame[thisIndex] > 0.0f) && (!fixZeros || scratchFrame2[thisIndex] != 0.0f))
                        {
                            float weight = kweights[kkern - k + krange];
                            weightsum += weight;
                            sum += weight * scratchFrame2[thisIndex];
                        }
                    }
                    if (weightsum != 0.0f)
                    {
                        scratchFrame[inVol->getIndex(i, j, k)] = sum / weightsum;
                    } else {
                        scratchFrame[inVol->getIndex(i, j, k)] = 0.0f;
                    }
                } else {
                    scratchFrame[inVol->getIndex(i, j, k)] = 0.0f;
                }
            }
        }
    }
}

void AlgorithmVolumeSmoothing::smoothFrameNonOrth(const float* inFrame, const vector<int64_t>& myDims, CaretArray<float>& scratchFrame, const VolumeFile* inVol, const VolumeFile* roiVol, const CaretArray<float**>& weights, const int& irange, const int& jrange, const int& krange, const bool& fixZeros)
{
    const float* roiFrame = NULL;
    if (roiVol != NULL)
    {
        roiFrame = roiVol->getFrame();
    }
#pragma omp CARET_PARFOR schedule(dynamic)
    for (int k = 0; k < myDims[2]; ++k)
    {
        for (int j = 0; j < myDims[1]; ++j)
        {
            for (int i = 0; i < myDims[0]; ++i)
            {
                if (roiVol == NULL || roiVol->getValue(i, j, k) > 0.0f)
                {
                    int imin = i - irange, imax = i + irange + 1;//one-after array size convention
                    if (imin < 0) imin = 0;
                    if (imax > myDims[0]) imax = myDims[0];
                    int jmin = j - jrange, jmax = j + jrange + 1;
                    if (jmin < 0) jmin = 0;
                    if (jmax > myDims[1]) jmax = myDims[1];
                    int kmin = k - krange, kmax = k + krange + 1;
                    if (kmin < 0) kmin = 0;
                    if (kmax > myDims[2]) kmax = myDims[2];
                    float sum = 0.0f, weightsum = 0.0f;
                    for (int kkern = kmin; kkern < kmax; ++kkern)
                    {
                        int64_t kindpart = kkern * myDims[1];
                        int kkernpart = kkern - k + krange;
                        for (int jkern = jmin; jkern < jmax; ++jkern)
                        {
                            int64_t jindpart = (kindpart + jkern) * myDims[0];
                            int jkernpart = jkern - j + jrange;
                            for (int ikern = imin; ikern < imax; ++ikern)
                            {
                                int64_t thisIndex = jindpart + ikern;//somewhat optimized index computation, could remove some integer multiplies, but there aren't that many
                                float weight = weights[kkernpart][jkernpart][ikern - i + irange];
                                if (weight != 0.0f && (roiVol == NULL || roiFrame[thisIndex] > 0.0f) && (!fixZeros || inFrame[thisIndex] != 0.0f))
                                {
                                    weightsum += weight;
                                    sum += weight * inFrame[thisIndex];
                                }
                            }
                        }
                    }
                    if (weightsum != 0.0f)
                    {
                        scratchFrame[inVol->getIndex(i, j, k)] = sum / weightsum;
                    } else {
                        scratchFrame[inVol->getIndex(i, j, k)] = 0.0f;
                    }
                } else {
                    scratchFrame[inVol->getIndex(i, j, k)] = 0.0f;
                }
            }
        }
    }
}

float AlgorithmVolumeSmoothing::getAlgorithmInternalWeight()
{
    return 1.0f;//override this if needed, if the progress bar isn't smooth
}

float AlgorithmVolumeSmoothing::getSubAlgorithmWeight()
{
    return 0.0f;
}
