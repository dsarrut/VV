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
#include "clitkSegmentationUtils.h"
#include "clitkExtractSliceFilter.h"
#include "clitkResampleImageWithOptionsFilter.h"

// itk
#include <itkJoinSeriesImageFilter.h>

//--------------------------------------------------------------------
template <class ImageType>
clitk::SliceBySliceRelativePositionFilter<ImageType>::
SliceBySliceRelativePositionFilter():
  clitk::FilterBase(),
  itk::ImageToImageFilter<ImageType, ImageType>()
{
  this->SetNumberOfRequiredInputs(2);
  SetDirection(2);
  SetObjectBackgroundValue(0);  
  SetFuzzyThreshold(0.6);
  SetOrientationTypeString("Left");
  SetIntermediateSpacing(10);
  ResampleBeforeRelativePositionFilterOff();
  UniqueConnectedComponentBySliceOff();
  NotFlagOff();
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template <class ImageType>
void 
clitk::SliceBySliceRelativePositionFilter<ImageType>::
SetInput(const ImageType * image) 
{
  // Process object is not const-correct so the const casting is required.
  this->SetNthInput(0, const_cast<ImageType *>(image));
}
//--------------------------------------------------------------------
  

//--------------------------------------------------------------------
template <class ImageType>
void 
clitk::SliceBySliceRelativePositionFilter<ImageType>::
SetInputObject(const ImageType * image) 
{
  // Process object is not const-correct so the const casting is required.
  this->SetNthInput(1, const_cast<ImageType *>(image));
}
//--------------------------------------------------------------------
  

//--------------------------------------------------------------------
template <class ImageType>
void 
clitk::SliceBySliceRelativePositionFilter<ImageType>::
GenerateInputRequestedRegion() 
{
  // Call default
  itk::ImageToImageFilter<ImageType, ImageType>::GenerateInputRequestedRegion();
  // Get input pointers and set requested region to common region
  ImagePointer input1 = dynamic_cast<ImageType*>(itk::ProcessObject::GetInput(0));
  ImagePointer input2 = dynamic_cast<ImageType*>(itk::ProcessObject::GetInput(1));
  input1->SetRequestedRegion(input1->GetLargestPossibleRegion());
  input2->SetRequestedRegion(input2->GetLargestPossibleRegion());
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template <class ImageType>
void 
clitk::SliceBySliceRelativePositionFilter<ImageType>::
GenerateOutputInformation() 
{
  // Get input pointer
  input = dynamic_cast<ImageType*>(itk::ProcessObject::GetInput(0));
  object = dynamic_cast<ImageType*>(itk::ProcessObject::GetInput(1));

  //--------------------------------------------------------------------
  // Resample object to the same spacing than input
  if (!clitk::HaveSameSpacing<ImageType, ImageType>(object, input)) {
    StartNewStep("Resample object to the same spacing than input");
    m_working_object = clitk::ResampleImageSpacing<ImageType>(object, input->GetSpacing());
    StopCurrentStep<ImageType>(m_working_object);
  }
  else {
    m_working_object = object;
  }
  
  //--------------------------------------------------------------------
  // Pad object to the same size than input
  if (!clitk::HaveSameSizeAndSpacing<ImageType, ImageType>(m_working_object, input)) {
    StartNewStep("Pad object to the same size than input");
    m_working_object = clitk::ResizeImageLike<ImageType>(m_working_object, 
                                                          input, 
                                                          GetObjectBackgroundValue());
    StopCurrentStep<ImageType>(m_working_object);
  }
  else {
  }

  /*
    - extract vector of slices in input, in object
    - slice by slice rel position
    - joint result
    - post process
  */


  //--------------------------------------------------------------------
  // Extract input slices
  StartNewStep("Extract input slices");
  typedef clitk::ExtractSliceFilter<ImageType> ExtractSliceFilterType;
  typename ExtractSliceFilterType::Pointer extractSliceFilter = ExtractSliceFilterType::New();
  extractSliceFilter->SetInput(input);
  extractSliceFilter->SetDirection(GetDirection());
  extractSliceFilter->Update();
  typedef typename ExtractSliceFilterType::SliceType SliceType;
  std::vector<typename SliceType::Pointer> mInputSlices;
  extractSliceFilter->GetOutputSlices(mInputSlices);
  StopCurrentStep<SliceType>(mInputSlices[0]);
  
  //--------------------------------------------------------------------
  // Extract object slices
  StartNewStep("Extract object slices");
  extractSliceFilter = ExtractSliceFilterType::New();
  extractSliceFilter->SetInput(m_working_object);//object);
  extractSliceFilter->SetDirection(GetDirection());
  extractSliceFilter->Update();
  std::vector<typename SliceType::Pointer> mObjectSlices;
  extractSliceFilter->GetOutputSlices(mObjectSlices);
  StopCurrentStep<SliceType>(mObjectSlices[0]);

  //--------------------------------------------------------------------
  // Perform slice by slice relative position
  StartNewStep("Perform slice by slice relative position");
  for(unsigned int i=0; i<mInputSlices.size(); i++) {
    // Select main CC in each object slice (required ?)
    mObjectSlices[i] = Labelize<SliceType>(mObjectSlices[i], 0, true, 1);
    mObjectSlices[i] = KeepLabels<SliceType>(mObjectSlices[i], 0, 1, 1, 1, true);

    // Relative position
    typedef clitk::AddRelativePositionConstraintToLabelImageFilter<SliceType> RelPosFilterType;
    typename RelPosFilterType::Pointer relPosFilter = RelPosFilterType::New();
    relPosFilter->VerboseStepOff();
    relPosFilter->WriteStepOff();
    relPosFilter->SetCurrentStepBaseId(this->GetCurrentStepId());
    relPosFilter->SetInput(mInputSlices[i]); 
    relPosFilter->SetInputObject(mObjectSlices[i]); 
    relPosFilter->SetNotFlag(GetNotFlag());
    relPosFilter->SetOrientationTypeString(this->GetOrientationTypeString());
    relPosFilter->SetIntermediateSpacing(this->GetIntermediateSpacing());
    relPosFilter->SetResampleBeforeRelativePositionFilter(this->GetResampleBeforeRelativePositionFilter());
    relPosFilter->SetFuzzyThreshold(this->GetFuzzyThreshold());
    relPosFilter->AutoCropFlagOff(); // important ! because we join the slices after this loop
    relPosFilter->Update();
    mInputSlices[i] = relPosFilter->GetOutput();

    // Select main CC if needed
    if (GetUniqueConnectedComponentBySlice()) {
      mInputSlices[i] = Labelize<SliceType>(mInputSlices[i], 0, true, 1);
      mInputSlices[i] = KeepLabels<SliceType>(mInputSlices[i], 0, 1, 1, 1, true);
    }

  }

  typedef itk::JoinSeriesImageFilter<SliceType, ImageType> JoinSeriesFilterType;
  typename JoinSeriesFilterType::Pointer joinFilter = JoinSeriesFilterType::New();
  joinFilter->SetOrigin(input->GetOrigin()[GetDirection()]);
  joinFilter->SetSpacing(input->GetSpacing()[GetDirection()]);
  for(unsigned int i=0; i<mInputSlices.size(); i++) {
    joinFilter->PushBackInput(mInputSlices[i]);
  }
  joinFilter->Update();
  m_working_input = joinFilter->GetOutput();
  StopCurrentStep<ImageType>(m_working_input);

  //--------------------------------------------------------------------
  // Step 7: autocrop
  if (GetAutoCropFlag()) {
    StartNewStep("Final AutoCrop");
    typedef clitk::AutoCropFilter<ImageType> CropFilterType;
    typename CropFilterType::Pointer cropFilter = CropFilterType::New();
    cropFilter->SetInput(m_working_input);
    cropFilter->ReleaseDataFlagOff();
    cropFilter->Update();   
    m_working_input = cropFilter->GetOutput();
    StopCurrentStep<ImageType>(m_working_input);    
  }

  // Update output info
  this->GetOutput(0)->SetRegions(m_working_input->GetLargestPossibleRegion());  
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template <class ImageType>
void 
clitk::SliceBySliceRelativePositionFilter<ImageType>::
GenerateData() 
{
  // Get input pointer
  //--------------------------------------------------------------------
  //--------------------------------------------------------------------  
  // Final Step -> set output
  //this->SetNthOutput(0, m_working_input);
  this->GraftOutput(m_working_input);
  return;
}
//--------------------------------------------------------------------

