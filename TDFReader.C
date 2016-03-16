void TDFReader(const char *file="run17029059.1.dat", TString outFilename="test.root" ,int nEvents=100000000)
{
	cout << "Reading "<<file<<" up to "<<nEvents<<" events\n";

    gROOT->Macro("Load.C");
	gSystem->Load("StIOMaker");
	gSystem->Load("StTriggerDataMaker");
	gSystem->Load("StMTDTrgMaker");
	cout << "Setting up chain" << endl;

	StChain* chain = new StChain;

	StIOMaker* iomaker = new StIOMaker("IO","r", file);
	StTriggerDataMaker* trgmaker = new StTriggerDataMaker();
	trgmaker->setDebug(0);

	StMTDTrgMaker* mtdtrgmaker = new StMTDTrgMaker();
	mtdtrgmaker->setOutFileName(outFilename);

	cout << "Init Chain" << endl;
	chain->Init();

	cout << "Event Loop  nEvents=" << nEvents << endl;
	//chain->EventLoop(0,nEvents);
	for(Int_t i=0; i<nEvents; i++){
		chain->Clear();
		Int_t iret = chain->Make(i);
		if(iret){
			cout<<"Bad return code!"<<endl;
			break;
		}
	}

	cout << "Finish Chain" << endl;
	chain->Finish();
	delete chain;
}
