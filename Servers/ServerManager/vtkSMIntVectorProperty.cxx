/*=========================================================================

  Program:   ParaView
  Module:    vtkSMIntVectorProperty.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMIntVectorProperty.h"

#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkPVXMLElement.h"

#include <vtkstd/vector>

vtkStandardNewMacro(vtkSMIntVectorProperty);
vtkCxxRevisionMacro(vtkSMIntVectorProperty, "1.9");

struct vtkSMIntVectorPropertyInternals
{
  vtkstd::vector<int> Values;
};

//---------------------------------------------------------------------------
vtkSMIntVectorProperty::vtkSMIntVectorProperty()
{
  this->Internals = new vtkSMIntVectorPropertyInternals;
  this->ArgumentIsArray = 0;
}

//---------------------------------------------------------------------------
vtkSMIntVectorProperty::~vtkSMIntVectorProperty()
{
  delete this->Internals;
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::AppendCommandToStream(
  vtkSMProxy*, vtkClientServerStream* str, vtkClientServerID objectId )
{
  if (!this->Command || this->IsReadOnly)
    {
    return;
    }

  if (!this->RepeatCommand)
    {
    *str << vtkClientServerStream::Invoke << objectId << this->Command;
    int numArgs = this->GetNumberOfElements();
    if (this->ArgumentIsArray)
      {
      *str << vtkClientServerStream::InsertArray(
        &(this->Internals->Values[0]), numArgs);
      }
    else
      {
      for(int i=0; i<numArgs; i++)
        {
        *str << this->GetElement(i);
        }
      }
    *str << vtkClientServerStream::End;
    }
  else
    {
    int numArgs = this->GetNumberOfElements();
    int numCommands = numArgs / this->NumberOfElementsPerCommand;
    for(int i=0; i<numCommands; i++)
      {
      *str << vtkClientServerStream::Invoke << objectId << this->Command;
      if (this->UseIndex)
        {
        *str << i;
        }
      if (this->ArgumentIsArray)
        {
        *str << vtkClientServerStream::InsertArray(
          &(this->Internals->Values[i*this->NumberOfElementsPerCommand]),
          this->NumberOfElementsPerCommand);
        }
      else
        {
        for (int j=0; j<this->NumberOfElementsPerCommand; j++)
          {
          *str << this->GetElement(i*this->NumberOfElementsPerCommand+j);
          }
        }
      *str << vtkClientServerStream::End;
      }
    }
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::SetNumberOfElements(unsigned int num)
{
  this->Internals->Values.resize(num);
  this->Modified();
}

//---------------------------------------------------------------------------
unsigned int vtkSMIntVectorProperty::GetNumberOfElements()
{
  return this->Internals->Values.size();
}

//---------------------------------------------------------------------------
int vtkSMIntVectorProperty::GetElement(unsigned int idx)
{
  return this->Internals->Values[idx];
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::SetElement(unsigned int idx, int value)
{
  if (this->IsReadOnly)
    {
    return;
    }

  if (idx >= this->GetNumberOfElements())
    {
    this->SetNumberOfElements(idx+1);
    }
  this->Internals->Values[idx] = value;
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::SetElements1(int value0)
{
  this->SetElement(0, value0);
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::SetElements2(int value0, int value1)
{
  this->SetElement(0, value0);
  this->SetElement(1, value1);
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::SetElements3(int value0, int value1, int value2)
{
  this->SetElement(0, value0);
  this->SetElement(1, value1);
  this->SetElement(2, value2);
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::SetElements(const int* values)
{
  if (this->IsReadOnly)
    {
    return;
    }
  int numArgs = this->GetNumberOfElements();
  memcpy(&this->Internals->Values[0], values, numArgs*sizeof(int));
  this->Modified();
}

//---------------------------------------------------------------------------
int vtkSMIntVectorProperty::ReadXMLAttributes(vtkPVXMLElement* element)
{
  int retVal;

  retVal = this->Superclass::ReadXMLAttributes(element);
  if (!retVal)
    {
    return retVal;
    }

  int arg_is_array;
  retVal = element->GetScalarAttribute("argument_is_array", &arg_is_array);
  if(retVal) 
    { 
    this->SetArgumentIsArray(arg_is_array); 
    }

  int numElems = this->GetNumberOfElements();
  if (numElems > 0)
    {
    int* initVal = new int[numElems];
    int numRead = element->GetVectorAttribute("default_values",
                                              numElems,
                                              initVal);

    if (numRead > 0)
      {
      if (numRead != numElems)
        {
        vtkErrorMacro("The umber of default values does not match the number "
                      "of elements. Initialization failed.");
        delete[] initVal;
        return 0;
        }
      this->SetElements(initVal);
      }
    delete[] initVal;
    }

  return 1;
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::SaveState(
  const char* name, ofstream* file, vtkIndent indent)
{
  unsigned int size = this->GetNumberOfElements();
  *file << indent << "<Property name=\"" << (this->XMLName?this->XMLName:"")
        << "\" id=\"" << name << "\" ";
  if (size > 0)
    {
    *file << "number_of_elements=\"" << size << "\"";
    }
  *file << ">" << endl;
  for (unsigned int i=0; i<size; i++)
    {
    *file << indent.GetNextIndent() << "<Element index=\""
          << i << "\" " << "value=\"" << this->GetElement(i) << "\"/>"
          << endl;
    }
  *file << indent << "</Property>" << endl;
}

//---------------------------------------------------------------------------
void vtkSMIntVectorProperty::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ArgumentIsArray: " << this->ArgumentIsArray << endl;
}
