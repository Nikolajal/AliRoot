#ifndef ALIEMCALPID_H
#define ALIEMCALPID_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */
                            
/* $Id$ */

//_________________________________________________________________________
//  Algorithm class for the identification of particles detected in EMCAL        
//  base  class                             
//  of identified particles                
//*-- Author: Yves Schutz (SUBATECH)

// --- ROOT system ---

#include "TTask.h" 
class TFormula ;
class TClonesArray ;

// --- Standard library ---

// --- AliRoot header files ---

class AliEMCALGeometry ;
class AliEMCALClusterizer ;
class AliEMCALTrackSegmentMaker ;

class AliEMCALPID : public TTask {

 public:

  AliEMCALPID() ;          // ctor            
  AliEMCALPID(const char* headerFile,const char * name, const Bool_t toSplit) ;
  virtual ~AliEMCALPID() ; // dtor

  virtual void Exec(Option_t * option) { Warning("Exec", "not defined" ) ; }
  virtual const Int_t GetRecParticlesInRun()  const { Warning("GetRecParticlesInRun", "not defined" ) ; return 0 ;} 
  virtual void Print(Option_t * option) const { Warning("Print", "not defined" ) ;}
  virtual void SetCpvtoEmcDistanceCut(Float_t Cluster_En, TString Eff_Pur,Float_t cut ) { Warning("SetCpvtoEmcDistanceCut", "not defined" ) ;}
  virtual void SetTimeGate(Float_t Cluster_En, TString Eff_Pur, Float_t gate) { Warning("SetTimeGate", "not defined" ) ; }
  virtual const char * Version() const { Warning("Version", "not defined" ) ; return 0 ; }  
  virtual void WriteRecParticles(Int_t event) { Warning("WriteRecParticles", "not defined" ) ; }

private: 
  virtual void Init() { Warning("Init", "not defined" ) ; } 

protected:

  TFile * fSplitFile ;             //! file in which RecParticles will eventually be stored
  Bool_t  fToSplit   ;             //! do we in the split mode  
  ClassDef(AliEMCALPID,1)  // Particle Identifier algorithm (base class)

} ;

#endif // ALIEMCALPID_H
