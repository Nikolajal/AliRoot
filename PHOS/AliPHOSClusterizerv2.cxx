/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
/* $Id:  */

/* $Log:
 */
//*-- Author: Boris Polichtchouk, IHEP
//////////////////////////////////////////////////////////////////////////////
//  Clusterization class for IHEP reconstruction.
// Performs clusterization (collects neighbouring active cells)
// It differs from AliPHOSClusterizerv1 in neighbour definition only

// --- ROOT system ---
#include "TBenchmark.h"
#include "TROOT.h"

// --- Standard library ---

// --- AliRoot header files ---
#include "AliPHOSClusterizerv2.h"
#include "AliPHOSGetter.h"
#include "TFolder.h"
#include "AliPHOSEvalRecPoint.h"
#include "AliPHOSRecCpvManager.h"
#include "AliPHOSRecEmcManager.h"

ClassImp(AliPHOSClusterizerv2)

AliPHOSClusterizerv2::AliPHOSClusterizerv2() : AliPHOSClusterizerv1() 
{}

AliPHOSClusterizerv2::AliPHOSClusterizerv2(const char * headerFile, const char * name, const Bool_t toSplit):
AliPHOSClusterizerv1(headerFile,name,toSplit)
{}

void AliPHOSClusterizerv2::GetNumberOfClustersFound(int* numb) const
{
  // Returns the number of found EMC and CPV rec.points

  AliPHOSGetter * gime = AliPHOSGetter::GetInstance() ;   
  numb[0] = gime->EmcRecPoints()->GetEntries();  
  numb[1] = gime->CpvRecPoints()->GetEntries();  
}

void AliPHOSClusterizerv2::Exec(Option_t* option)
{
  // Steering method

  if(strstr(option,"tim"))
    gBenchmark->Start("PHOSClusterizer"); 
  
  if(strstr(option,"print"))
    Print("") ; 

  AliPHOSGetter * gime = AliPHOSGetter::GetInstance() ; 

  TFolder* storage = dynamic_cast<TFolder*>(gROOT->FindObjectAny("Folders/Run/Event/RecData/RecPoints/PHOS")); 
  TFolder* wPoolF =  storage->AddFolder("SmP","SmartRecPoints for PHOS");
  
  TObjArray* wPool = new TObjArray(400);
  wPool->SetName("SmartPoints");
  wPoolF->Add(wPool);
  wPoolF->Add(this);

  Int_t nevents = (Int_t) gAlice->TreeE()->GetEntries() ;
  Int_t ievent ;

  for(ievent = 0; ievent<nevents; ievent++) {
    
    gAlice->GetEvent(ievent) ;
    gAlice->SetEvent(ievent) ;
    
    gime->Event(ievent,"D") ;
//      if(!ReadDigits(ievent))  //reads digits for event fEvent
//        continue;
    
    Info("Exec", "MakeClusters invoked..") ;
    MakeClusters() ;
    Info("Exec", "MakeClusters done.") ;


    //SmartRecPoints will communicate with wPool.

    AliPHOSEvalRecPoint* rp=0;

    // CPV reconstruction

    AliPHOSRecCpvManager* recCpv = new AliPHOSRecCpvManager();
    wPoolF->Add(recCpv);

    Int_t iPoint; //loop variable

    for(iPoint=0; iPoint<gime->CpvRecPoints()->GetEntriesFast(); iPoint++) {
      rp = new AliPHOSEvalRecPoint(iPoint,AliPHOSEvalRecPoint::kCpv);
      rp->MakeJob();
    }

    AliPHOSEvalRecPoint pt;
    pt.UpdateWorkingPool();

    TObjArray * cpvRecPoints = gime->CpvRecPoints() ; 
    Int_t nOldCpv = cpvRecPoints->GetEntries();
    cpvRecPoints->Delete();
    cpvRecPoints->Compress();

    Int_t i; //loop variable

    for(i=0; i<wPool->GetEntries(); i++)
      cpvRecPoints->Add(wPool->At(i));

    wPool->Clear();
    wPool->Compress();

    wPoolF->Remove(recCpv);
    delete recCpv;

    Info("Exec", "       %d", gime->CpvRecPoints()->GetEntries() ) ;
    Info("Exec", "       %d cpvRecPoints", cpvRecPoints->GetEntries() ) ;


    // Now Emc reconstruction

    AliPHOSRecEmcManager* recEmc = new AliPHOSRecEmcManager();
    wPoolF->Add(recEmc);

    for(iPoint=0; iPoint<gime->EmcRecPoints()->GetEntriesFast(); iPoint++) {
      rp = new AliPHOSEvalRecPoint(iPoint,(Bool_t)AliPHOSEvalRecPoint::kEmc);
      rp->MakeJob();
    }

    pt.UpdateWorkingPool();

    TObjArray * emcRecPoints = gime->EmcRecPoints() ; 
    Int_t nOldEmc = emcRecPoints->GetEntries();
    emcRecPoints->Delete();
    emcRecPoints->Compress();

    for(i=0; i<wPool->GetEntries(); i++)
      emcRecPoints->Add(wPool->At(i));

    wPool->Clear();
    wPool->Compress();

    wPoolF->Remove(recEmc);
    delete recEmc;

    TString message ; 
    message  = "       %d  OLD cpvRecPoints\n" ; 
    message += "       %d\n" ; 
    message += "       %d cpvRecPoints\n" ; 

    message += "       %d OLD emcRecPoints " ; 
    message += "       %d\n" ;
    message += "       %d emcRecPoints\n" ;

    Info("Exec", message.Data(), 
	 nOldCpv, 
	 gime->CpvRecPoints()->GetEntries(),cpvRecPoints->GetEntries(), 
	 nOldEmc, 
	 gime->EmcRecPoints()->GetEntries(), emcRecPoints->GetEntries() ) ; 

    WriteRecPoints(ievent);


  } // loop over events

  if(strstr(option,"tim")) {
    gBenchmark->Stop("PHOSClusterizer");
    Info("Exec","took %f seconds for Clusterizing", gBenchmark->GetCpuTime("PHOSClusterizer") ) ;
  }
}
//---------------------------------------------------------------------------------
Int_t AliPHOSClusterizerv2::AreNeighbours(AliPHOSDigit* d1, AliPHOSDigit* d2) const
{
  // Points are neighbours if they have common edge.
  // Points with common vertex are NOT neighbours.
  // This treatment of neighbourship is the part of 
  // IHEP algorithm of clusterization.

  // Gives the neighbourness of two digits = 0 are not neighbour but continue searching 
  //                                       = 1 are neighbour
  //                                       = 2 are not neighbour but do not continue searching
  // The order of d1 and d2 is important: first (d1) should be a digit already in a cluster 
  // which is compared to a digit (d2)  not yet in a cluster  

  const AliPHOSGeometry * geom = AliPHOSGetter::GetInstance()->PHOSGeometry();

  Int_t rv = 0 ; 

  Int_t relid1[4] ; 
  geom->AbsToRelNumbering(d1->GetId(), relid1) ; 

  Int_t relid2[4] ; 
  geom->AbsToRelNumbering(d2->GetId(), relid2) ; 
 
  if ( (relid1[0] == relid2[0]) && (relid1[1]==relid2[1]) ) { // inside the same PHOS module and the same PPSD Module 
    Int_t rowdiff = TMath::Abs( relid1[2] - relid2[2] ) ;  
    Int_t coldiff = TMath::Abs( relid1[3] - relid2[3] ) ;  
    
    if ( ( (coldiff < 1) && (rowdiff <= 1) ) || ( ( coldiff <= 1 )  && ( rowdiff < 1 ) ) ){
      rv = 1 ; 
    }
    else {
      if((relid2[2] > relid1[2]) && (relid2[3] > relid1[3]+1)) 
	rv = 2; //  Difference in row numbers is too large to look further 
    }

  } 
  else {
    
    if( (relid1[0] < relid2[0]) || (relid1[1] < relid2[1]) )  
      rv=2 ;

  }

  return rv ; 

}
