/*=========================================================================

  Program:   ParaView
  Module:    vtkSMProxyClipboard.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMProxyClipboard - helper class to help copy/paste properties on
// proxies.
// .SECTION Description
// vtkSMProxyClipboard is a helper class used to enable copy/paste for
// properties on proxies. Copy/Paste is more that simply copying values from a
// proxy to another (for which one should simply use vtkSMProxy::Copy()). This
// has several extra smarts including the following:
// \li Skips input properties.
// \li Skips properties that are hidden by setting
// vtkSMProperty::PanelVisibility to "never".
// \li Specially handles properties with vtkSMProxyListDomain to select the same
// type of proxy from the target domain when pasting and then loading the
// state for the value proxy separately.
// \li For other vtkSMProxyProperty instances (e.g. LookupTable,
// ScalarOpacityFunction, etc.) on Paste, it tries to locate the requested proxy
// value on the session.

#ifndef vtkSMProxyClipboard_h
#define vtkSMProxyClipboard_h

#include "vtkPVServerManagerDefaultModule.h" //needed for exports
#include "vtkSMObject.h"
#include "vtkSmartPointer.h" // for vtkSmartPointer.

class vtkPVXMLElement;
class vtkSMProxy;
class vtkSMProxyClipboardInternals;

class VTKPVSERVERMANAGERDEFAULT_EXPORT vtkSMProxyClipboard : public vtkSMObject
{
public:
  static vtkSMProxyClipboard* New();
  vtkTypeMacro(vtkSMProxyClipboard, vtkSMObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Put a proxy's state on the clipboard. Return true if the operation was
  // successful.
  bool Copy(vtkSMProxy* source);

  // Description:
  // Returns true of the proxy state on the clipboard can be pasted on to the
  // specified \c target. Current implementation doesn't check for the types of the
  // proxies. In future, we can make this type-aware or add other criteria.
  bool CanPaste(vtkSMProxy* target);

  // Description:
  // Paste the state on the clipboard on to the target proxy.
  bool Paste(vtkSMProxy* target);

  // Description:
  // Clears the clipboard. Same as calling Copy(NULL).
  void Clear()
    { this->Copy(NULL); }

protected:
  vtkSMProxyClipboard();
  ~vtkSMProxyClipboard();

  vtkSmartPointer<vtkPVXMLElement> CopiedState;

private:
  vtkSMProxyClipboard(const vtkSMProxyClipboard&); // Not implemented
  void operator=(const vtkSMProxyClipboard&); // Not implemented

  vtkSMProxyClipboardInternals* Internals;

};

#endif
