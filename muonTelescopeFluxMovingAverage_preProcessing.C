/*

  Name:           muonTelescopeFluxMovingAverageRm402SA.C
  Created by:   Hexc, Ernesto and Arfa
  Date:             July 2, 2020 

  Updated:      July 21, 2020
                       Added histograms for the counts in each channel (layer).

  Updated by:  Hexc, Ernesto and Arfa
  Date:             Aug 6, 2020
                        Added code to start counting muons at the beginning of first full hour recording. 

  Purpose:        Graph the timeseries from muon telescope flux (in percent change) using moving average

  To run it, do:

  - Crate a file test.dat example format per line given below
  - start ROOT
  root [0] .L muonTelescopeMovingAverageRm402SA.C+
  root [1] muonTelescopeFluxMovingAverageRm402SA("xxx.log", nhour);

*/

#include <string.h>
#include <stdio.h>
#include "TFile.h"
#include "TTree.h"
#include "TString.h"  
#include "TVectorD.h"
#include "TH1.h"
#include "TF1.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TTimeStamp.h"
#include "TLegend.h"
#include "TStyle.h"
#include "Getline.h"
#include <time.h>

#include <iostream>
#include <fstream>

// #define compile

using namespace std;

/*-----------------------------------------------------------------------------*/

void muonTelescopeFluxMovingAverage_preProcessing(string filename, int nHours) {
  
  char rootfile[256];
  int i, j, ch, n, chn_index;
  double t1, t2, dt;
  
  TDatime da(2003,02,28,12,00,00);
  gStyle->SetTimeOffset(da.Convert());
  
  vector <double> timeAxis;
  vector <double> timeAxis_hr;

  vector <double> zeroAndOne; 
  vector <double> zeroAndTwo;  
  vector <double> zeroAndThree;
  vector <double> oneAndTwo;  
  vector <double> oneAndThree;
  vector <double> twoAndThree;

  vector <double> zeroAndOne_hr; 
  vector <double> zeroAndTwo_hr;  
  vector <double> zeroAndThree_hr;

  vector <double> oneAndTwo_hr, oneAndTwo_hr_pct, oneAndTwo_hr_pct_mAve;  
  vector <double> oneAndThree_hr, oneAndThree_hr_pct, oneAndThree_hr_pct_mAve;
  vector <double> twoAndThree_hr, twoAndThree_hr_pct, twoAndThree_hr_pct_mAve;
  
  vector <double> zero;
  vector <double> one;
  vector <double> two;
  vector <double> three;

  double oneAndTwo_hr_ave = 0.;
  double oneAndThree_hr_ave = 0.;
  double twoAndThree_hr_ave = 0.;

  // The list of elements for labeling
  vector <string> elements;
  
  // A 2D array containg all the data
  vector < vector<double> > data;
  
#ifndef compile
  const int events = 100;
  const int eventInfo = 10;
#else
  const int events = 1000000;
  const int eventInfo = 1000;
#endif
  
  // Open the csv data file
  FILE* f = fopen(Form("%s", filename.c_str()), "r");
  if (f == NULL) {
    printf("Cannot find file \'%s\'\n", filename.c_str());
    return;
  }
  
  // open the root file
  strcpy(rootfile, filename.c_str());
  if (strchr(rootfile, '.'))
    *strchr(rootfile, '.') = 0;
  strcat(rootfile, ".root");
  TFile* outfile = new TFile(Form("Running%dHours_%s", nHours, rootfile), "RECREATE");
  
  // Buffer for each line
  char line[128];
  
  // Read header
  fgets(line, sizeof(line), f);
  
  cout << line << endl;
  
  int counter = 0;
  int lastHour = 0, currentHour = 0, totalHours = 0;
  int ch12_hr = 0, ch13_hr = 0, ch23_hr = 0;
  int ch12, ch13, ch23;
  int year, month, day, hour, minute, second, nanosecond;
  int check00minute = 1;     // initialize this value to 1 so that we can find the first 00 minute mark
  
  //Added histograms for coincidence 13, 23
  auto h12 = new TH1F("h12","Ch12 Coincidence (hourly)", 1000, 10000, 15000);
  auto h13 = new TH1F("h13","Ch13 Coincidence (hourly)", 1800,   1000, 10000);
  auto h23 = new TH1F("h23","Ch23 Coincidence (hourly)", 1000, 10000, 15000);

  auto hCh1 = new TH1F("hCh1","Ch1 counts (each minute)", 1000, 0, 2000);
  auto hCh2 = new TH1F("hCh2","Ch2 counts (each minute)", 1000, 0, 2000);
  auto hCh3 = new TH1F("hCh3","Ch3 counts (each minute)", 1000, 0, 2000);
  

/*
#include <iostream>
#include <fstream>
using namespace std;

int main () {
  ofstream myfile;
  myfile.open ("example.txt");
  myfile << "Writing this to a file.\n";
  myfile.close();
  return 0;
}
*/

   ofstream myOutput;
   myOutput.open("hourlyCounts.txt");

  //Read all lines until the end
  while(fgets(line, sizeof(line), f) != NULL){
    if (!(counter%1000)) cout << line << "  " << counter << endl;
    
    //Timestamp
    char * timestampStr = strtok(line, ",");
    
    zeroAndOne.push_back(atoi(strtok(NULL, ",")));
    zeroAndTwo.push_back(atoi(strtok(NULL, ",")));
    zeroAndThree.push_back(atoi(strtok(NULL, ",")));

    ch12 = atoi(strtok(NULL, ","));
    oneAndTwo.push_back(ch12);
    ch13 = atoi(strtok(NULL, ","));
    oneAndThree.push_back(ch13);
    ch23 = atoi(strtok(NULL, ","));
    twoAndThree.push_back(ch23);
    
    zero.push_back(atoi(strtok(NULL, ",")));
    one.push_back(atoi(strtok(NULL, ",")));
    two.push_back(atoi(strtok(NULL, ",")));
    three.push_back(atoi(strtok(NULL, ",")));
    
    year        = atoi(strtok(timestampStr, "-"));
    month       = atoi(strtok(NULL, "-"));
    day         = atoi(strtok(NULL, "T"));
    hour        = atoi(strtok(NULL, ":"));

    minute      = atoi(strtok(NULL, ":"));
    if (check00minute>0) {
      cout << " minute: " << minute << endl;
      if (minute == 0) check00minute = 0;
      continue;
    }
    
    counter++;     // This is the right place to start counting

    second      = atoi(strtok(NULL, "."));
    nanosecond  = atoi(strtok(NULL, "Z")) * 1000000;

    TTimeStamp * timestamp = new TTimeStamp(year,
                                            month,
                                            day,
                                            hour,
                                            minute,
                                            second,
					    nanosecond);
    // cumulate hourly counts
   if (counter == 1) {
      lastHour = hour;
    }
    if (hour == lastHour) {
      ch12_hr += ch12;
      ch13_hr += ch13;
      ch23_hr += ch23;
    } else {
      lastHour = hour;
      totalHours++;
      oneAndTwo_hr.push_back(ch12_hr);
      oneAndThree_hr.push_back(ch13_hr);
      twoAndThree_hr.push_back(ch23_hr);
      timeAxis.push_back(timestamp->AsDouble());
      oneAndTwo_hr_ave = oneAndTwo_hr_ave + (ch12_hr - oneAndTwo_hr_ave)/totalHours;
      oneAndThree_hr_ave = oneAndThree_hr_ave + (ch13_hr - oneAndThree_hr_ave)/totalHours;
      twoAndThree_hr_ave = twoAndThree_hr_ave + (ch23_hr - twoAndThree_hr_ave)/totalHours;
      h12->Fill(ch12_hr);
      h13->Fill(ch13_hr);
      h23->Fill(ch23_hr);
      myOutput << ch12_hr << "   "  << ch13_hr << "   " << ch23_hr << endl;
      ch12_hr = ch12;
      ch13_hr = ch13;
      ch23_hr = ch23;
      cout << " Month: " << month << "   Hour: " << hour << endl;
    }
    
    // printf("%s\n", timestamp->AsString()); 
    // timeAxis.push_back(timestamp->AsDouble());

  }

    
  fclose(f);
  myOutput.close();
  
  cout << " Total hours: " << totalHours << endl;
  cout << " Time points: " << timeAxis.size() << endl;
  cout << " ave Ch12_hr: " << oneAndTwo_hr_ave << "   ave Ch13_hr:  " << oneAndThree_hr_ave << "   ave Ch23_hr:  " << twoAndThree_hr_ave << endl;

  // Book the histograms of minute-counts in each layer
  // This is helpful for checking out the potential problem in each layer
  for (int jj = 0; jj<one.size(); jj++)
    {
      hCh1->Fill(one[jj]);
      hCh2->Fill(two[jj]);
      hCh3->Fill(three[jj]);
    }

  
  // Calculate the percent change of the muon flux coincidence
  for (int iv = 0; iv<oneAndTwo_hr.size(); iv++){
    oneAndTwo_hr_pct.push_back((oneAndTwo_hr[iv] - oneAndTwo_hr_ave)/oneAndTwo_hr_ave);
    oneAndThree_hr_pct.push_back((oneAndThree_hr[iv] - oneAndThree_hr_ave)/oneAndThree_hr_ave);
    twoAndThree_hr_pct.push_back((twoAndThree_hr[iv] - twoAndThree_hr_ave)/twoAndThree_hr_ave);
  }
  
  // Fill the moving average data array
  for (int iv = 0; iv<oneAndTwo_hr.size(); iv++){
    if (iv < nHours) continue;
    // calculate the average in nHours starting from when iv index reaches to nHours
    double movingAve12 = 0.0, movingAve13 = 0.0, movingAve23 = 0.0;
    for (int ihr = 0; ihr < nHours; ihr++) {
      movingAve12 += (oneAndTwo_hr_pct[iv-ihr-1] - movingAve12)/(ihr+1);
      movingAve13 += (oneAndThree_hr_pct[iv-ihr-1] - movingAve13)/(ihr+1);
      movingAve23 += (twoAndThree_hr_pct[iv-ihr-1] - movingAve23)/(ihr+1);
    }
    oneAndTwo_hr_pct_mAve.push_back(movingAve12);
    oneAndThree_hr_pct_mAve.push_back(movingAve13);
    twoAndThree_hr_pct_mAve.push_back(movingAve23);
  }

  // Define the plotting canvas
  TCanvas *c1 = new TCanvas("c1","Time Series Plots",200,10,1480,800);
  c1->Divide(1,3);
  
  // Define the singleChannel graph
  
  // Define the coincidence graph
  TGraph* oneAndTwoGraph     = new TGraph(timeAxis.size(), &(timeAxis[0]), &(oneAndTwo_hr_pct[0]));
  TGraph* oneAndThreeGraph   = new TGraph(timeAxis.size(), &(timeAxis[0]), &(oneAndThree_hr_pct[0]));
  TGraph* twoAndThreeGraph   = new TGraph(timeAxis.size(), &(timeAxis[0]), &(twoAndThree_hr_pct[0]));

  TGraph* g12mAve = new TGraph(timeAxis.size()-nHours, &(timeAxis[nHours]), &(oneAndTwo_hr_pct_mAve[0]));
  TGraph* g13mAve = new TGraph(timeAxis.size()-nHours, &(timeAxis[nHours]), &(oneAndThree_hr_pct_mAve[0]));
  TGraph* g23mAve = new TGraph(timeAxis.size()-nHours, &(timeAxis[nHours]), &(twoAndThree_hr_pct_mAve[0]));
    
  c1->cd(1);
  
  oneAndTwoGraph->SetLineColor(kBlue);
  oneAndTwoGraph->SetTitle("");
  oneAndTwoGraph->GetXaxis()->SetTimeDisplay(1);
  oneAndThreeGraph->GetXaxis()->SetNdivisions(50709);
  //  oneAndTwoGraph->GetXaxis()->SetTimeFormat("%Y-%m-%d %H:%M");
  oneAndTwoGraph->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  oneAndTwoGraph->GetXaxis()->SetTimeOffset(0,"gmt");
  oneAndTwoGraph->GetYaxis()->SetRangeUser(-0.10, 0.10);
  oneAndTwoGraph->GetXaxis()->SetLabelSize(0.08);
  oneAndTwoGraph->GetYaxis()->SetLabelSize(0.08);
  oneAndTwoGraph->SetMarkerStyle(27);
  oneAndTwoGraph->SetMarkerColor(kBlue);
  oneAndTwoGraph->GetXaxis()->SetTitle("Time&Date");
  oneAndTwoGraph->GetYaxis()->SetTitle("Flux Percent Change");
  oneAndTwoGraph->GetXaxis()->SetTitleSize(0.08);
  //  oneAndTwoGraph->GetXaxis()->CenterTitle(true);
  oneAndTwoGraph->GetYaxis()->SetTitleSize(0.08);
  oneAndTwoGraph->GetYaxis()->SetTitleOffset(0.30);
  oneAndTwoGraph->GetXaxis()->SetTitleOffset(0.30);

  
  //oneAndThreeGraph->SetTitle("Two-Layer Coincidence (1&3)");
  
  twoAndThreeGraph->SetLineColor(kRed);  
  twoAndThreeGraph->SetMarkerStyle(20);
  twoAndThreeGraph->SetMarkerColor(kRed);
  twoAndThreeGraph->SetTitle("");
  twoAndThreeGraph->GetXaxis()->SetTimeDisplay(1);
  twoAndThreeGraph->GetXaxis()->SetNdivisions(50709);
  //  twoAndThreeGraph->GetXaxis()->SetTimeFormat("%Y-%m-%d %H:%M");
  twoAndThreeGraph->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  twoAndThreeGraph->GetXaxis()->SetTimeOffset(0,"gmt");
  twoAndThreeGraph->GetYaxis()->SetRangeUser(-0.10, 0.10);
  twoAndThreeGraph->GetXaxis()->SetLabelSize(0.08);
  twoAndThreeGraph->GetYaxis()->SetLabelSize(0.08);  
  twoAndThreeGraph->SetLineColor(kRed);
  twoAndThreeGraph->GetXaxis()->SetTitle("Time&Date");
  twoAndThreeGraph->GetYaxis()->SetTitle("Flux Percent Change");
  twoAndThreeGraph->GetXaxis()->SetTitleSize(0.08);
  //  twoAndThreeGraph->GetXaxis()->CenterTitle(true);
  twoAndThreeGraph->GetYaxis()->SetTitleSize(0.08);
  twoAndThreeGraph->GetYaxis()->SetTitleOffset(0.30);
  twoAndThreeGraph->GetXaxis()->SetTitleOffset(0.30);
 

  oneAndThreeGraph->SetMarkerStyle(22);
  oneAndThreeGraph->SetMarkerColor(kGreen);
  //twoAndThreeGraph->SetTitle("Two-Layer Coincidence (2&3)");  
  oneAndThreeGraph->SetLineColor(kGreen);
  oneAndThreeGraph->SetTitle("");
  oneAndThreeGraph->GetXaxis()->SetTimeDisplay(1);
  oneAndThreeGraph->GetXaxis()->SetNdivisions(50709);
  //  oneAndThreeGraph->GetXaxis()->SetTimeFormat("%Y-%m-%d %H:%M");
  oneAndThreeGraph->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  oneAndThreeGraph->GetXaxis()->SetTimeOffset(0,"gmt");
  oneAndThreeGraph->GetYaxis()->SetRangeUser(-0.10, 0.10);
  oneAndThreeGraph->GetXaxis()->SetLabelSize(0.08);
  oneAndThreeGraph->GetYaxis()->SetLabelSize(0.08);  
  oneAndThreeGraph->SetLineColor(kGreen);
  oneAndThreeGraph->GetXaxis()->SetTitle("Time&Date");
  oneAndThreeGraph->GetYaxis()->SetTitle("Flux Percent Change");
  oneAndThreeGraph->GetXaxis()->SetTitleSize(0.08);
  //  oneAndThreeGraph->GetXaxis()->CenterTitle(true);
  oneAndThreeGraph->GetYaxis()->SetTitleSize(0.08);
  oneAndThreeGraph->GetYaxis()->SetTitleOffset(0.30);
  oneAndThreeGraph->GetXaxis()->SetTitleOffset(0.30);
  

  //  TMultiGraph  *mg  = new TMultiGraph();
  //  mg->Add(oneAndTwoGraph);
  //  mg->Add(twoAndThreeGraph);
  //  mg->Add(oneAndThreeGraph);
  //  mg->Draw("APC");

  oneAndTwoGraph->Draw("AP");
  g12mAve->SetLineWidth(3);
  g12mAve->Draw("same");
  
  // Add a legend for labeling the graphs  
  TLegend *legend12 = new TLegend(0.3,0.15,0.6,0.25); 
  
  legend12->AddEntry(oneAndTwoGraph,"Rm402SA(home) hourly counts (ch12)", "l");     // other options are available, e.g.,  "f"
  legend12->AddEntry(g12mAve, Form("Moving average: %d hours", nHours), "l");
  legend12->SetBorderSize(0);
  legend12->SetNColumns(2);
  legend12->Draw();

  c1->cd(2);
  twoAndThreeGraph->Draw("AP");
  g23mAve->SetLineWidth(3);
  g23mAve->Draw("same");

  // Add a legend for labeling the graphs  
  TLegend *legend23 = new TLegend(0.3,0.15,0.6,0.25); 
  
  legend23->AddEntry(twoAndThreeGraph,"Rm402SA(home) hourly counts (ch23)", "l");     // other options are available, e.g.,  "f"
  legend23->AddEntry(g23mAve, Form("Moving average: %d hours", nHours), "l");
  legend23->SetBorderSize(0);
  legend23->SetNColumns(2);
  legend23->Draw();

  c1->cd(3);
  oneAndThreeGraph->Draw("AP");
  g13mAve->SetLineWidth(3);
  g13mAve->Draw("same");

  // Add a legend for labeling the graphs  
  TLegend *legend13 = new TLegend(0.3,0.15,0.6,0.25); 
  
  legend13->AddEntry(oneAndThreeGraph,"Rm402SA(home) hourly counts (ch13)", "l");     // other options are available, e.g.,  "f"
  legend13->AddEntry(g13mAve, Form("Moving average: %d hours", nHours), "l");
  legend13->SetBorderSize(0);
  legend13->SetNColumns(2);
  legend13->Draw();
  
  /*
    TLegend * legend = new TLegend(0.69,0.45,0.8,0.65);
    // legend->SetHeader("Channels","C"); // option "C" allows to center the header
    legend->AddEntry(zeroGraph,"Channel 0","l");
    legend->AddEntry(oneGraph,"Channel 1","l");
    legend->AddEntry(twoGraph,"Channel 2","l");
    legend->AddEntry(threeGraph,"Channel 3","l");
    legend->Draw();
  */
  
  
  c1->Update();

  // Draw counts histogram
  /*
  TCanvas *c2 = new TCanvas("c2","Histograms of hourly counts",200,10,1480,800);
  c2->cd(1);
  h23->Draw();
  h12->Draw("same");
  h13->Draw("same");
  c2->Update();
  */

  /*
  h12->Write();
  h13->Write();
  h23->Write();
  */

  //  sleep(2);

  //coincidenceGraphs->Write();
  // The following line causes a problem of displaying c2  1/28/2020
  //outfile->Close();
  h12->Write("h12");
  h13->Write("h13");
  h23->Write("h23");
  hCh1->Write("hCh1");
  hCh2->Write("hCh2");
  hCh3->Write("hCh3");
  oneAndTwoGraph->Write("g12");
  oneAndThreeGraph->Write("g13");
  twoAndThreeGraph->Write("g23");
  g12mAve->Write("g12mAve");
  g13mAve->Write("g13mAve");
  g23mAve->Write("g23mAve");  
  
}	 
