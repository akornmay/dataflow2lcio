// This test program reads dataflow simuation output from text file
// and write it out in LCIO format
//
// Compile with included make file and execute after sourcing "env.sh"
//
//
// Adapted from a program of 
// A.F.Zarnecki   March 2007
// updated January 2008 for use with new simulation results
//
// By M. Delcourt, August 2014


// system includes

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <math.h>

// lcio includes

#include "lcio.h"
#include "IMPL/LCRunHeaderImpl.h"
#include "IMPL/LCEventImpl.h"
#include "IMPL/TrackerHitImpl.h"
#include "IMPL/SimTrackerHitImpl.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/TrackerDataImpl.h"
#include "UTIL/CellIDEncoder.h"

// gsl includes

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

//Other includes
#include "EUTelConvertSim.h"


using namespace std;
using namespace lcio;
using namespace CMSPixel;

string treeName = "hitTree"; //tree Name in the input ROOT file

int main(int argc, char ** argv) {


  // input and output file names

  string inputFileName  = (argc>1) ? argv[1]:"treeFile.root"; 
  string outputFileName = (argc>2) ? argv[2]:"outFile";

  cout << "Converting " << inputFileName.c_str()
       << " to "        <<  outputFileName.c_str() <<  endl;  

  // Prepare the output slcio file.
  
  LCWriter * lcWriter = LCFactory::getInstance()->createLCWriter();

  // open the file
  try {
    lcWriter->open(outputFileName.c_str(),LCIO::WRITE_NEW);
  }
  catch (IOException& e) {
    cerr << e.what() << endl;
    return -1;
  }
  
  // Get number of detectors
  
  int Ndet;
  if (argc > 3){
      Ndet = atoi(argv[3]);
  }
  else{
      Ndet=8;
  }
  cout << "Telescope setup with " << Ndet << " layers" <<  endl;

  
 // Prepare a run header

  int runNumber       = 1;
  int eventNumber     = 0;
  string detectorName = "CMSPixelTelescope";
  string detectorDescription = "CMSPixelTelescope (simulated data for testbeam)";

  LCRunHeaderImpl * runHeader = new LCRunHeaderImpl();
  runHeader->setRunNumber(runNumber);
  runHeader->setDetectorName(detectorName);
  runHeader->setDescription(detectorDescription);

  //Set plane names

  for(int idet=0; idet < Ndet; idet++)
     {
      stringstream detName;
      detName<<"Roc"<<idet;
     // Add plane names to run header

      runHeader->addActiveSubdetector(detName.str());
      }


  // write the header to the output file

  lcWriter->writeRunHeader(runHeader);

  // delete the run header since not used anymore

  delete runHeader;

  //Start the reader (from EUTelConvertSim.h)
  simReader * reader = new simReader(inputFileName,treeName);	
  int loopbreak=1;  
  //Creates charge vectors that will store hits
  vector <float> * charge = new vector <float> [Ndet];
  vector <float> * chargeSparse = new vector <float> [Ndet];
  //Event loop

  while ( loopbreak>0 ) {      //loopbreak = -1 if end of file, 1 if not.

	eventNumber++;
        if (eventNumber%1000 == 0) 
              cout << "Converting event number " << eventNumber << endl;
	
	// Prepare event header and collections 

	LCEventImpl * event = new LCEventImpl();

        event->setDetectorName(detectorName);
        event->setRunNumber(runNumber);
        event->setEventNumber(eventNumber);
	event->parameters().setValue("EventType", (int) 2 ) ;

	//Create 2 different collections
	LCCollectionVec     * simhitvec = new LCCollectionVec(LCIO::TRACKERDATA); 	//Raw data 
	LCCollectionVec     * simhitvecSparse = new LCCollectionVec(LCIO::TRACKERDATA); //Sparse data
	
	// Set cell ID encoding for both collections
	string cellIDEncoding( "sensorID:5,sparsePixelType:5");
	CellIDEncoder<TrackerDataImpl> b( cellIDEncoding , simhitvec ) ;
	CellIDEncoder<TrackerDataImpl> b2( cellIDEncoding , simhitvecSparse ) ;

	
	
	//Get event
	timing t;
	vector<pixel> p;
	//Read all events from root file with same timestamp
	//Stores hit in "pixel" structure and time in "timing" structure
	//Returns -1 at the end of the file to exit event loop
	loopbreak = reader->getSimulatedEvent(&p,t); 

// 	loopbreak= 1;//DEBUG WARNING

	event->setTimeStamp(t.timestamp);
	
	
	//Sort data from p in different ROCs
	
	for (int h=0; h<p.size(); h++){
	    int det = p[h].roc;
	    if (det > Ndet-1 || det <0)
		{cerr<<"roc="<<det<<" not between 0 and "<<Ndet-1<<"! event ignored."<<endl; continue;}
	    
	    charge[det].push_back(p[h].col);
	    charge[det].push_back(p[h].row);
	    charge[det].push_back(p[h].raw);
	    
	    chargeSparse[det].push_back(p[h].col);
	    chargeSparse[det].push_back(p[h].row);
	    chargeSparse[det].push_back(p[h].vcal);
	}
	
	//Write hits to collections for every ROC
	for(int det = 0; det < Ndet ; det++){
	    TrackerDataImpl * simhit = new TrackerDataImpl;
	    TrackerDataImpl * simhitSparse = new TrackerDataImpl;
    
	    simhit->setCellID0(0);
	    simhitSparse->setCellID0(0);
	    
	    //Set CellIDEncoder values (same for both collections)
	    b["sensorID"] = det ;
	    b["sparsePixelType"] = 1;
	    b.setCellID( simhit ) ;
	    b.setCellID( simhitSparse ) ;
	    
	    //Add charge deposition in both hits
	    simhit->setChargeValues(charge[det]);
	    simhitSparse->setChargeValues(chargeSparse[det]);
	    //Add hits to collection
	    simhitvec->push_back(simhit);
	    simhitvecSparse->push_back(simhitSparse);
	}
	//Add collection to event
	event->addCollection(simhitvec,"data");
	event->addCollection(simhitvecSparse,"sparse");
	
	// write event out
	lcWriter->writeEvent(event);
	
	
	// clean
	// deleting an event also delets everything what was put into this event...
	delete event;
	
	// clear charge vectors
	for (int i=0; i<Ndet; i++){
	    charge[i].clear();
	    chargeSparse[i].clear();
	}
	
// End of event loop
}


// That is all! Close all streams...

  
  lcWriter->close();

  cout<<"done"<<endl;
  return 0;
}

