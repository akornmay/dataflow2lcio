#include "EUTelConvertSim.h"
using namespace std;
namespace CMSPixel
{
    simReader::simReader(){
	init("treeFile.root","hitTree");
    }

    simReader::simReader(string filename, string treeName){
	init(filename,treeName);
    }
    
    void simReader::init(string filename, string treeName){
	evtNumber=0;
	treeFile = new TFile(filename.c_str());
	t = (TTree*) treeFile->Get(treeName.c_str());
	if (!t){cerr<<"Unable to open tree"<<endl; exit(0);}
	t->SetBranchAddress("TS",&d.TS);
	t->SetBranchAddress("roc",&d.roc);
	t->SetBranchAddress("row",&d.myrow);
	t->SetBranchAddress("col",&d.mycol);
	t->SetBranchAddress("vcal",&d.vcal);
	t->SetBranchAddress("pulseHeight",&d.pulseHeight);
	t->SetBranchAddress("phase",&d.phase);
	t->SetBranchAddress("trigger_number",&d.trigger_number);
	t->SetBranchAddress("token_number",&d.token_number);
	t->SetBranchAddress("triggers_stacked",&d.triggers_stacked);
	t->SetBranchAddress("trigger_phase",&d.trigger_phase);
	t->SetBranchAddress("data_phase",&d.data_phase);
	t->SetBranchAddress("status",&d.status);

	treeSize = t->GetEntries();
	
    }
    
    simReader::~simReader(){
	treeFile->Close();
    }
    
    int simReader::getSimulatedEvent(std::vector<pixel> * p, timing & time){
	 vector < pair <uint8_t,uint8_t> > unused;
	 return(getSimulatedEvent(p,&unused,time));
    }
   
    int simReader::getSimulatedEvent(std::vector<pixel> * p, std::vector<std::pair<uint8_t,uint8_t> > * readback, timing & time){
	pixel hit;
	t->GetEntry(evtNumber++);
	
	//get event timestamp
	TS = d.TS;
	
	//Write timing info ( one per event )
	time.timestamp=TS;
	time.trigger_number=d.trigger_number;
	time.token_number=d.token_number;
	time.triggers_stacked=d.triggers_stacked;
	time.trigger_phase=d.trigger_phase;
	time.data_phase=d.data_phase;
	time.status=d.status;
	
	//Get next events while the timestamp is still the same
	do{
	    //Hits
	    hit.roc = d.roc;
	    hit.col = d.mycol;
	    hit.row = d.myrow;
	    hit.raw = d.vcal;
	    hit.vcal= d.pulseHeight;
	    p->push_back(hit);	    
	    
	    
	    if(evtNumber==treeSize){evtNumber=0; return(-1);} //if end of file, return -1
	    t->GetEntry(evtNumber++);
	}while(TS==d.TS);
	//if next timestamp, go back one event
	evtNumber--;
	return(1);// if not end of file, return 1
    }
}
