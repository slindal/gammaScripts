AliAnalysisTaskParticleCorrelation *AddTaskPartCorr(TString inputDataType, TString calorimeter, Bool_t kPrintSettings = kFALSE,Bool_t kSimulation = kFALSE, Bool_t outputAOD=kFALSE, Bool_t oldAOD=kFALSE)
{
  // Creates a PartCorr task, configures it and adds it to the analysis manager.
  
  // Get the pointer to the existing analysis manager via the static access method.
  //==============================================================================
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    ::Error("AddTaskPartCorr", "No analysis manager to connect to.");
    return NULL;
  }  
 
  Bool_t kUseKinematics = kFALSE; 
  if(kSimulation) { 
    kUseKinematics = (mgr->GetMCtruthEventHandler())?kTRUE:kFALSE; 
    if (!kUseKinematics && inputDataType == "AOD") kUseKinematics = kTRUE; //AOD primary should be available ... 
  } 
  
  cout<<"********* ACCESS KINE? "<<kUseKinematics<<endl;
  
  // Configure analysis
  //===========================================================================
  
  // *** Reader ***
  AliCaloTrackReader * reader =0x0 ;
  if(inputDataType.Contains("AOD")) reader = new AliCaloTrackAODReader();
  else if(inputDataType=="ESD") reader = new AliCaloTrackESDReader();
  else if(inputDataType=="MC" && inputDataType == "ESD") reader = new AliCaloTrackMCReader();
  reader->SetDebug(-1);//10 for lots of messages
  reader->SwitchOnCTS();
  reader->SwitchOffEMCAL();
  reader->SwitchOffPHOS();
  reader->SwitchOffEMCALCells(); 
  reader->SwitchOffPHOSCells(); 
  

  reader->SetDataType(2);
    
  //Min particle pT
  reader->SetEMCALPtMin(0.3); 
  reader->SetPHOSPtMin(0.3);
  reader->SetCTSPtMin(0.1);
  reader->SwitchOnWriteDeltaAOD()  ;
  // if(oldAOD) reader->SwitchOnOldAODs();
  reader->SwitchOnWriteDeltaAOD();
  if(kPrintSettings) reader->Print("");
  
  // *** Calorimeters Utils	***
  AliCalorimeterUtils *cu = new AliCalorimeterUtils;
  
  
  //Recalibration
  //cu->SwitchOnRecalibration();
  //TFile * f = new TFile("RecalibrationFactors.root","read");
  //cu->SetEMCALChannelRecalibrationFactors(0,(TH2F*)f->Get("EMCALRecalFactors_SM0"));
  //cu->SetEMCALChannelRecalibrationFactors(1,(TH2F*)f->Get("EMCALRecalFactors_SM1"));
  //cu->SetEMCALChannelRecalibrationFactors(2,(TH2F*)f->Get("EMCALRecalFactors_SM2"));
  //cu->SetEMCALChannelRecalibrationFactors(3,(TH2F*)f->Get("EMCALRecalFactors_SM3"));
  //f->Close();	
  
  cu->SetDebug(-1);
  if(kPrintSettings) cu->Print("");
  
  
  // ##### Analysis algorithm settings ####
  
  
  //==================================
  // ### Isolation analysis ###	
  //=================================
  //Photon
  AliAnaParticleIsolation *anaisol = new AliAnaParticleIsolation();
  anaisol->SetDebug(-1);
  anaisol->SetMinPt(0);
  anaisol->SetInputAODName("ConversionGamma");
  anaisol->SetAODObjArrayName("ConversionGamma");
  anaisol->SetCalorimeter("CTS");
  
  if(kUseKinematics) anaisol->SwitchOnDataMC() ;//Access MC stack and fill more histograms
  else  anaisol->SwitchOffDataMC() ;

  anaisol->SwitchOffInvariantMass();

  AliIsolationCut * ic =  anaisol->GetIsolationCut();	
  ic->SetConeSize(0.4);
  ic->SetPtThreshold(0.7);
  ic->SetPtFraction(0.1);
  ic->SetSumPtThreshold(1.0) ;
  ic->SetParticleTypeInCone(AliIsolationCut::kOnlyCharged);
  ic->SetICMethod(AliIsolationCut::kSumPtFracIC);
  if(kPrintSettings) ic->Print("");

  anaisol->SwitchOnSeveralIsolation();
  anaisol->SetHistoPtRangeAndNBins(0, 50, 200) ;
  anaisol->AddToHistogramsName("AnaIsolPhoton_");
  
  anaisol->SetOutputAODClassName("AliAODPWG4ParticleCorrelation");
  

  anaisol->SwitchOnReIsolation();
  anaisol->SwitchOnSeveralIsolation();
  anaisol->SetPtThresholds(0, 1.0);
  anaisol->SetPtThresholds(1, 2.0);
  anaisol->SetPtThresholds(2, 3.0);
  anaisol->SetPtThresholds(3, 4.0);
  anaisol->SetPtThresholds(4, 5.0);


  anaisol->SetPtFractions(0, 0.4);
  anaisol->SetPtFractions(1, 0.3);
  anaisol->SetPtFractions(2, 0.1);
  anaisol->SetPtFractions(3, 0.08);
  anaisol->SetPtFractions(4, 0.05);

  anaisol->SetNCones(5);
  anaisol->SetConeSizes(0, 0.50);
  anaisol->SetConeSizes(1, 0.60);
  anaisol->SetConeSizes(2, 0.70);
  anaisol->SetConeSizes(3, 0.80);
  anaisol->SetConeSizes(4, 0.90);
  



  if(kPrintSettings) anaisol->Print("");
  
	
  // //===========================
  // //Correlation analysis
  // //===========================
	
  // // ### Correlation with Jet Finder AOD output
  AliAnaParticleJetFinderCorrelation *anacorrjet = new AliAnaParticleJetFinderCorrelation();
  anacorrjet->SetInputAODName("ConversionGamma");//,calorimeter.Data()));
  anacorrjet->SwitchOffFiducialCut();
  anacorrjet->SetDebug(-1);
  anacorrjet->SetConeSize(1);  
  anacorrjet->SelectIsolated(kTRUE); // do correlation with isolated photons
  anacorrjet->SetPtThresholdInCone(0.2);
  anacorrjet->SetDeltaPhiCutRange(0.5,5.5);//Mostly Open Cuts 
  anacorrjet->SetRatioCutRange(0.01,3); //Mostly Open Cuts
  anacorrjet->UseJetRefTracks(kFALSE); //Not working now
  //Set Histograms bins and ranges
  anacorrjet->SetHistoPtRangeAndNBins(0, 50, 200) ;
  //      ana->SetHistoPhiRangeAndNBins(0, TMath::TwoPi(), 100) ;
  //      ana->SetHistoEtaRangeAndNBins(-0.7, 0.7, 100) ;
  if(kPrintSettings) anacorrjet->Print("");
  
  
  // #### Configure Maker ####
  AliAnaPartCorrMaker * maker = new AliAnaPartCorrMaker();
  maker->SetReader(reader);//pointer to reader
  maker->SetCaloUtils(cu); //pointer to calorimeter utils
  Int_t n = 0;//Analysis number, order is important
  maker->AddAnalysis(anaisol,n++);
  maker->AddAnalysis(anacorrjet, n++);
  maker->SetAnaDebug(-1);
  maker->SwitchOnHistogramsMaker()  ;
 	
  if(kPrintSettings) maker->Print("");
  
  printf("======================== \n");
  printf(" End Configuration of PartCorr analysis with detector %s \n",calorimeter.Data());
  printf("======================== \n");
  
  // Create task
  //===========================================================================
  AliAnalysisTaskParticleCorrelation * task = new AliAnalysisTaskParticleCorrelation (Form("PartCorr%s",calorimeter.Data()));
  task->SetConfigFileName(""); //Don't configure the analysis via configuration file.
  task->SetDebugLevel(-1);
  task->SetAnalysisMaker(maker);
  if(inputDataType=="ESD" && !kSimulation) task->SelectCollisionCandidates(); //AliPhysicsSelection has to be attached before.
  mgr->AddTask(task);
  
  //Create containers
  char name[128];
  sprintf(name,"PartCorr_%s",calorimeter.Data());
  cout<<"Name of task "<<name<<endl;
  
  TString outputfile = AliAnalysisManager::GetCommonFileName(); 
  AliAnalysisDataContainer *cout_pc   = mgr->CreateContainer(calorimeter.Data(), TList::Class(), 
                                                             AliAnalysisManager::kOutputContainer, 
                                                             Form("%s:PartCorr",outputfile.Data()));
	
  AliAnalysisDataContainer *cout_cuts = mgr->CreateContainer(Form("%sCuts",calorimeter.Data()), TList::Class(), 
                                                             AliAnalysisManager::kParamContainer, 
                                                             Form("%s:PartCorrCuts",outputfile.Data()));
	
  // Create ONLY the output containers for the data produced by the task.
  // Get and connect other common input/output containers via the manager as below
  //==============================================================================
  mgr->ConnectInput  (task, 0, mgr->GetCommonInputContainer());
  // AOD output slot will be used in a different way in future
  if(!inputDataType.Contains("delta")   && outputAOD) mgr->ConnectOutput (task, 0, mgr->GetCommonOutputContainer());
  mgr->ConnectOutput (task, 1, cout_pc);
  mgr->ConnectOutput (task, 2, cout_cuts);
  
  return task;
}


