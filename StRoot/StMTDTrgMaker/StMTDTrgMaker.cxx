#include <stdlib.h>

#include "StMTDTrgMaker.h"
#include "StMaker.h"
#include "StEventTypes.h"
#include "StEvent/StTriggerData2016.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"

StBeamDirection eastwest(int ew) {
	if(ew==0) return east;
	return west;
};

ClassImp(StMTDTrgMaker)
StMTDTrgMaker::StMTDTrgMaker(const char *name) : StMaker(name), fOutFile(0), mOutFileName(""), mEvtTree(0), mEventCounter(0) 
{
	LOG_INFO << "Constructing StMTDTrgMaker with name=" << name << endm;
}

Int_t StMTDTrgMaker::Init()
{

	if(!mOutFileName.Length()){
		LOG_ERROR << "StMTDTrgMaker:: No output file specified for tree and histograms." << endm;
		return kStErr;
	}

	fOutFile =  new TFile(mOutFileName.Data(), "RECREATE"); 
	if(!fOutFile){
	    LOG_ERROR << "StMTDTrgMaker:: Failed to create the output file to store the tree and histograms." << endm;
		return kStErr;
	}

	BookTreeHist();
	
	return kStOK;
}

Int_t StMTDTrgMaker::Finish()
{
	LOG_INFO << "StMTDTrgMaker:: Finish()" << endm;
	LOG_INFO << "StMTDTrgMaker:: Processed " << mEventCounter << " events." << endm;  
	if(fOutFile){
		fOutFile->cd();
		fOutFile->Write();
		fOutFile->Close();
	}  
	return kStOK;
}

Int_t StMTDTrgMaker::Make()
{
	memset(&event,0,sizeof(event)); //initial the event

	mEventCounter ++ ;

	TObjectSet *os = (TObjectSet*)GetDataSet("StTriggerData");
	if(os){
		td = (StTriggerData2016*)os->GetObject();   
		if(!td){
			LOG_ERROR << "StMTDTrgMaker:: Could not get StTriggerData from DataSet." << endm;
			return kStErr;
		}
	}else{
		LOG_ERROR << "StMTDTrgMaker:: Could not get StTriggerData DataSet." << endm;
		return kStErr;
	}

	event.eventNumber            = td->eventNumber();
	event.npre                   = td->numberOfPreXing();                 
	event.npost                  = td->numberOfPostXing();
	event.nsum                   = event.npre + event.npost + 1;
	

	// cout << "Pre = " << event.npre << endl << endm;
	// cout << "Post = " << event.npost << endl << endm;

	Int_t npre1  = event.npre;
	Int_t npost1 = event.npost;
	for(Int_t i=-npre1; i<npost1+1; i++){
		Int_t prepost = i;

		for(Int_t j=0; j<16; j++){
			// old format - for old VPD alignment code
			if(j>=0 && j<8){
				event.Vpd[prepost+npre1][j]      = td->vpdADC(east,j+1,prepost);
				event.Vpd[prepost+npre1][j+8]    = td->vpdTDC(east,j+1,prepost);
				event.Vpd[prepost+npre1][j+32]   = td->vpdADC(west,j+1,prepost);
				event.Vpd[prepost+npre1][j+40]   = td->vpdTDC(west,j+1,prepost);

				event.VpdHi[prepost+npre1][j]    = td->vpdADCHighThr(east,j+1,prepost);
				event.VpdHi[prepost+npre1][j+8]  = td->vpdTDCHighThr(east,j+1,prepost);
				event.VpdHi[prepost+npre1][j+32] = td->vpdADCHighThr(west,j+1,prepost);
				event.VpdHi[prepost+npre1][j+40] = td->vpdTDCHighThr(west,j+1,prepost);
			}else{
				event.Vpd[prepost+npre1][j+8]    = td->vpdADC(east,j+1,prepost);
				event.Vpd[prepost+npre1][j+16]   = td->vpdTDC(east,j+1,prepost);
				event.Vpd[prepost+npre1][j+40]   = td->vpdADC(west,j+1,prepost);
				event.Vpd[prepost+npre1][j+48]   = td->vpdTDC(west,j+1,prepost);

				event.VpdHi[prepost+npre1][j+8]  = td->vpdADCHighThr(east,j+1,prepost);
				event.VpdHi[prepost+npre1][j+16] = td->vpdTDCHighThr(east,j+1,prepost);
				event.VpdHi[prepost+npre1][j+40] = td->vpdADCHighThr(west,j+1,prepost);
				event.VpdHi[prepost+npre1][j+48] = td->vpdTDCHighThr(west,j+1,prepost);
			}

			// new format - the same format as the MTD QT;
			event.bbqEastVpd[prepost+npre1][j+j/4*4]   = td->vpdADC(east,j+1,prepost); 
			event.bbqEastVpd[prepost+npre1][j+j/4*4+4] = td->vpdTDC(east,j+1,prepost); 
			event.bbqWestVpd[prepost+npre1][j+j/4*4]   = td->vpdADC(west,j+1,prepost); 
			event.bbqWestVpd[prepost+npre1][j+j/4*4+4] = td->vpdTDC(west,j+1,prepost); 

			event.mxqEastVpd[prepost+npre1][j+j/4*4]   = td->vpdADCHighThr(east,j+1,prepost); 
			event.mxqEastVpd[prepost+npre1][j+j/4*4+4] = td->vpdTDCHighThr(east,j+1,prepost); 
			event.mxqWestVpd[prepost+npre1][j+j/4*4]   = td->vpdADCHighThr(west,j+1,prepost); 
			event.mxqWestVpd[prepost+npre1][j+j/4*4+4] = td->vpdTDCHighThr(west,j+1,prepost); 
		}
		event.fastBbqEastVpd[prepost+npre1] = td->vpdEarliestTDC(east, prepost);
		event.fastBbqWestVpd[prepost+npre1] = td->vpdEarliestTDC(west, prepost);
		event.fastMxqEastVpd[prepost+npre1] = td->vpdEarliestTDCHighThr(east, prepost);
		event.fastMxqWestVpd[prepost+npre1] = td->vpdEarliestTDCHighThr(west, prepost);


		event.tofMult[prepost+npre1]        = td->tofMultiplicity(prepost);
		for(Int_t j=0; j<120; j++){
			event.tofTrayMult[prepost+npre1][j] = td->tofTrayMultiplicity(j+1, prepost);
		}

		for(Int_t j=0; j<32; j++){
			event.MT001[prepost+npre1][j]    = td->mtdQtAtCh(1,j,prepost);
			event.MT002[prepost+npre1][j]    = td->mtdQtAtCh(2,j,prepost);
			event.MT003[prepost+npre1][j]    = td->mtdQtAtCh(3,j,prepost);
			event.MT004[prepost+npre1][j]    = td->mtdQtAtCh(4,j,prepost);
			event.MT005[prepost+npre1][j]    = td->mtdQtAtCh(5,j,prepost);
			event.MT006[prepost+npre1][j]    = td->mtdQtAtCh(6,j,prepost);
			event.MT007[prepost+npre1][j]    = td->mtdQtAtCh(7,j,prepost);
			event.MT008[prepost+npre1][j]    = td->mtdQtAtCh(8,j,prepost);

			event.MtdDsm[prepost+npre1][j]   = td->mtdDsmAtCh(j,prepost);
		}

		for(Int_t j=0; j<8; j++){
			event.DsmTF201Ch[j]   = td->dsmTF201Ch(j);
			event.lastDsmBit[j]   = td->lastDSM(j);
		}


		// event.sumBbqEastVpd[prepost+npre1] = td->vpdSumTac( east, prepost );
		// event.sumBbqWestVpd[prepost+npre1] = td->vpdSumTac( west, prepost );

		// event.sumADCBbqEastVpd[prepost+npre1] = td->vpdAdcSum( east, prepost );
		// event.sumADCBbqWestVpd[prepost+npre1] = td->vpdAdcSum( west, prepost );

		// event.nGoodBbqEastVpd[prepost+npre1] = td->vpdNGoodHit( east, prepost );
		// event.nGoodBbqWestVpd[prepost+npre1] = td->vpdNGoodHit( west, prepost );

		event.bbcEastVpdNHits[prepost+npre1] = (td->bbcVP101(4, prepost)>>12); 
        event.bbcEastVpdADCsum[prepost+npre1] = (td->bbcVP101(4, prepost)&0xfff);
        event.bbcEastVpdTACsum[prepost+npre1] = td->bbcVP101(5, prepost);
        event.bbcWestVpdNHits[prepost+npre1] = (td->bbcVP101(6, prepost)>>12); 
        event.bbcWestVpdADCsum[prepost+npre1] = (td->bbcVP101(6, prepost)&0xfff);
        event.bbcWestVpdTACsum[prepost+npre1] = td->bbcVP101(7, prepost);

	}




	mEvtTree->Fill();

	Int_t address[8] = {0, 9, 10, 11, 12, 13, 14, 15}; // 8 MTD QT borads
	for(Int_t i=0;i<8;i++){
		for(Int_t j=0;j<16;j++){
			Int_t adcIdx = j+j/4*4;
			Int_t tacIdx = j+j/4*4+4;
			hMxqMtdADC[i][j]->Fill(td->mxqAtSlotAddress(adcIdx,0,address[i]));
			hMxqMtdTAC[i][j]->Fill(td->mxqAtSlotAddress(tacIdx,0,address[i]));
		}
	}



	

	return kStOK;
}

void StMTDTrgMaker::BookTreeHist()
{
	LOG_INFO << "StMTDTrgMaker:: Book the event tree to be filled." << endm;
	mEvtTree = new TTree("trgEvent","trgEvent");
	mEvtTree->SetAutoSave(100000000); // 100 MB

	mEvtTree->Branch("eventNumber", &event.eventNumber, "eventNumber/i");
	mEvtTree->Branch("npre",&event.npre,"npre/i");
	mEvtTree->Branch("npost",&event.npost,"npost/i");
	mEvtTree->Branch("nsum",&event.nsum,"nsum/i");
	mEvtTree->Branch("Vpd",&event.Vpd,"Vpd[nsum][64]/s");
	mEvtTree->Branch("VpdHi",&event.VpdHi,"VpdHi[nsum][64]/s");
	mEvtTree->Branch("bbqEastVpd",&event.bbqEastVpd,"bbqEastVpd[nsum][32]/s");
	mEvtTree->Branch("bbqWestVpd",&event.bbqWestVpd,"bbqWestVpd[nsum][32]/s");
	mEvtTree->Branch("fastBbqEastVpd",&event.fastBbqEastVpd,"fastBbqEastVpd[nsum]/s");
	mEvtTree->Branch("fastBbqWestVpd",&event.fastBbqWestVpd,"fastBbqWestVpd[nsum]/s");

	mEvtTree->Branch("sumBbqEastVpd",&event.sumBbqEastVpd,"sumBbqEastVpd[nsum]/s");
	mEvtTree->Branch("sumBbqWestVpd",&event.sumBbqWestVpd,"sumBbqWestVpd[nsum]/s");
	mEvtTree->Branch("sumADCBbqEastVpd",&event.sumADCBbqEastVpd,"sumADCBbqEastVpd[nsum]/s");
	mEvtTree->Branch("sumADCBbqWestVpd",&event.sumADCBbqWestVpd,"sumADCBbqWestVpd[nsum]/s");
	mEvtTree->Branch("nGoodBbqEastVpd",&event.nGoodBbqEastVpd,"nGoodBbqEastVpd[nsum]/s");
	mEvtTree->Branch("nGoodBbqWestVpd",&event.nGoodBbqWestVpd,"nGoodBbqWestVpd[nsum]/s");


	mEvtTree->Branch("bbcEastVpdNHits",&event.bbcEastVpdNHits,"bbcEastVpdNHits[nsum]/s");
	mEvtTree->Branch("bbcWestVpdNHits",&event.bbcWestVpdNHits,"bbcWestVpdNHits[nsum]/s");
	mEvtTree->Branch("bbcEastVpdADCsum",&event.bbcEastVpdADCsum,"bbcEastVpdADCsum[nsum]/s");
	mEvtTree->Branch("bbcWestVpdADCsum",&event.bbcWestVpdADCsum,"bbcWestVpdADCsum[nsum]/s");
	mEvtTree->Branch("bbcEastVpdTACsum",&event.bbcEastVpdTACsum,"bbcEastVpdTACsum[nsum]/s");
	mEvtTree->Branch("bbcWestVpdTACsum",&event.bbcWestVpdTACsum,"bbcWestVpdTACsum[nsum]/s");

	mEvtTree->Branch("mxqEastVpd",&event.mxqEastVpd,"mxqEastVpd[nsum][32]/s");
	mEvtTree->Branch("mxqWestVpd",&event.mxqWestVpd,"mxqWestVpd[nsum][32]/s");
	mEvtTree->Branch("fastMxqEastVpd",&event.fastMxqEastVpd,"fastMxqEastVpd[nsum]/s");
	mEvtTree->Branch("fastMxqWestVpd",&event.fastMxqWestVpd,"fastMxqWestVpd[nsum]/s");
	mEvtTree->Branch("tofMult",&event.tofMult,"tofMult[nsum]/s");
	mEvtTree->Branch("tofTrayMult",&event.tofTrayMult,"tofTrayMult[nsum][120]/s");
	mEvtTree->Branch("MT001",&event.MT001,"MT001[nsum][32]/s");
	mEvtTree->Branch("MT002",&event.MT002,"MT002[nsum][32]/s");
	mEvtTree->Branch("MT003",&event.MT003,"MT003[nsum][32]/s");
	mEvtTree->Branch("MT004",&event.MT004,"MT004[nsum][32]/s");
	mEvtTree->Branch("MT005",&event.MT005,"MT005[nsum][32]/s");
	mEvtTree->Branch("MT006",&event.MT006,"MT006[nsum][32]/s");
	mEvtTree->Branch("MT007",&event.MT007,"MT007[nsum][32]/s");
	mEvtTree->Branch("MT008",&event.MT008,"MT008[nsum][32]/s");
	mEvtTree->Branch("MtdDsm",&event.MtdDsm,"MtdDsm[nsum][32]/b");
	mEvtTree->Branch("DsmTF201Ch",&event.DsmTF201Ch,"DsmTF201Ch[8]/s");
	mEvtTree->Branch("lastDsmBit",&event.lastDsmBit,"lastDsmBit[8]/s");

	for(Int_t i=0; i<8; i++){
		for(Int_t j=0; j<16; j++){
			hMxqMtdADC[i][j] = new TH1D(Form("MT00%d_ADC%d",i+1,j+1),Form("MT00%d_ADC%d",i+1,j+1),2048,0,4096);
			hMxqMtdTAC[i][j] = new TH1D(Form("MT00%d_TAC%d",i+1,j+1),Form("MT00%d_TAC%d",i+1,j+1),2048,0,4096);
		}
	}
}
