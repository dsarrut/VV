/*=========================================================================
  Program:   vv                     http://www.creatis.insa-lyon.fr/rio/vv

  Authors belong to: 
  - University of LYON              http://www.universite-lyon.fr/
  - Léon Bérard cancer center       http://oncora1.lyon.fnclcc.fr
  - CREATIS CNRS laboratory         http://www.creatis.insa-lyon.fr

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the copyright notices for more information.

  It is distributed under dual licence

  - BSD        See included LICENSE.txt file
  - CeCILL-B   http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
======================================================================-====*/

// clitk
#include "clitkResampleImage_ggo.h"
#include "clitkIO.h"
#include "clitkResampleImageGenericFilter.h"

//--------------------------------------------------------------------
int main(int argc, char * argv[]) {

  // Init command line
  GGO(clitkResampleImage, args_info);

  // Filter
  typedef clitk::ResampleImageGenericFilter<args_info_clitkResampleImage> FilterType;
  FilterType::Pointer filter = FilterType::New();
  
  filter->SetArgsInfo(args_info);
  filter->Update();

  // this is the end my friend  
  return EXIT_SUCCESS;

}// end main
//--------------------------------------------------------------------