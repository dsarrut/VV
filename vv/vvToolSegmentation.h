/*=========================================================================
  Program:   vv                     http://www.creatis.insa-lyon.fr/rio/vv

  Authors belong to: 
  - University of LYON              http://www.universite-lyon.fr/
  - Léon Bérard cancer center       http://www.centreleonberard.fr
  - CREATIS CNRS laboratory         http://www.creatis.insa-lyon.fr

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the copyright notices for more information.

  It is distributed under dual licence

  - BSD        See included LICENSE.txt file
  - CeCILL-B   http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
===========================================================================**/
#ifndef VVTOOLSEGMENTATION_H
#define VVTOOLSEGMENTATION_H

#include <QtDesigner/QDesignerExportWidget>

#include "vvToolBase.h"
#include "vvToolWidgetBase.h"
#include "vvROIActor.h"
#include "ui_vvToolSegmentation.h"

#include "vtkLookupTable.h"

//------------------------------------------------------------------------------
class vvToolSegmentation:
  public vvToolWidgetBase,
  public vvToolBase<vvToolSegmentation>, 
  private Ui::vvToolSegmentation 
{
  Q_OBJECT
    public:
  vvToolSegmentation(vvMainWindowBase * parent=0, Qt::WindowFlags f=0);
  ~vvToolSegmentation();

  //-----------------------------------------------------
  static void Initialize();
  virtual void InputIsSelected(vvSlicerManager * m);
  void OpenBinaryImage();
  void Erode();
  void Dilate();
  void Labelize();
  void RemoveLabel();
  void UpdateAndRenderNewMask();

  //-----------------------------------------------------
  public slots:
  virtual void apply();
  bool eventFilter(QObject *object, QEvent *event);
  virtual bool close();
  virtual void MousePositionChanged(int slicer);
  //  virtual void keyPressEvent(QKeyEvent * event);
  // virtual void reject();

 protected:
  // virtual void closeEvent(QCloseEvent *event);
  Ui::vvToolSegmentation ui;
  QSharedPointer<vvROIActor> mRefMaskActor;
  QSharedPointer<vvROIActor> mCurrentMaskActor;
  std::vector<QSharedPointer<vvROIActor> > mCurrentCCLActors;
  vvImage::Pointer mRefMaskImage;
  vvImage::Pointer mCurrentMaskImage;
  vvImage::Pointer mCurrentCCLImage;
  int mKernelValue;
  vtkSmartPointer<vtkLookupTable> mDefaultLUTColor;
  enum { Mode_Default, Mode_CCL};
  int mCurrentMode;

  QSharedPointer<vvROIActor> CreateMaskActor(vvImage::Pointer image, int i, int colorID, bool BGMode=false);

}; // end class vvToolSegmentation
//------------------------------------------------------------------------------

#endif