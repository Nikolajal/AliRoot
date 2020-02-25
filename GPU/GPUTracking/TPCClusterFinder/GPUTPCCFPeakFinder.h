//**************************************************************************\
//* This file is property of and copyright by the ALICE Project            *\
//* ALICE Experiment at CERN, All rights reserved.                         *\
//*                                                                        *\
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *\
//*                  for The ALICE HLT Project.                            *\
//*                                                                        *\
//* Permission to use, copy, modify and distribute this software and its   *\
//* documentation strictly for non-commercial purposes is hereby granted   *\
//* without fee, provided that the above copyright notice appears in all   *\
//* copies and that both the copyright notice and this permission notice   *\
//* appear in the supporting documentation. The authors make no claims     *\
//* about the suitability of this software for any purpose. It is          *\
//* provided "as is" without express or implied warranty.                  *\
//**************************************************************************

/// \file PeakFinder.h
/// \author Felix Weiglhofer

#ifndef O2_GPU_PEAK_FINDER_H
#define O2_GPU_PEAK_FINDER_H

#include "GPUGeneralKernels.h"
#include "GPUConstantMem.h"

#include "clusterFinderDefs.h"
#include "Array2D.h"
#include "PackedCharge.h"

namespace GPUCA_NAMESPACE
{
namespace gpu
{

class GPUTPCCFPeakFinder : public GPUKernelTemplate
{

 public:
  struct GPUSharedMemory : public GPUKernelTemplate::GPUSharedMemoryScan64<short, GPUCA_THREAD_COUNT_CLUSTERER> {
    ChargePos posBcast[SCRATCH_PAD_WORK_GROUP_SIZE];
    PackedCharge buf[SCRATCH_PAD_WORK_GROUP_SIZE * SCRATCH_PAD_SEARCH_N];
  };

  enum K : int {
    findPeaks,
  };

  static GPUd() void findPeaksImpl(int, int, int, int, GPUSharedMemory&, const Array2D<gpu::PackedCharge>&, const deprecated::Digit*, uint, uchar*, Array2D<uchar>&);

#ifdef HAVE_O2HEADERS
  typedef GPUTPCClusterFinder processorType;
  GPUhdi() static processorType* Processor(GPUConstantMem& processors)
  {
    return processors.tpcClusterer;
  }
#endif

  GPUhdi() CONSTEXPR static GPUDataTypes::RecoStep GetRecoStep()
  {
    return GPUDataTypes::RecoStep::TPCClusterFinding;
  }

  template <int iKernel = defaultKernel, typename... Args>
  GPUd() static void Thread(int nBlocks, int nThreads, int iBlock, int iThread, GPUSharedMemory& smem, processorType& clusterer, Args... args);

 private:
  static GPUd() bool isPeakScratchPad(GPUSharedMemory&, Charge, const ChargePos&, ushort, const Array2D<o2::gpu::PackedCharge>&, ChargePos*, PackedCharge*);

  static GPUd() bool isPeak(Charge, const ChargePos&, const Array2D<PackedCharge>&);
};

} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif
