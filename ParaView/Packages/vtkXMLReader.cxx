/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXMLReader.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkXMLReader.h"

#include "vtkDataArray.h"
#include "vtkDataCompressor.h"
#include "vtkDataSet.h"
#include "vtkInstantiator.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataParser.h"
#include "vtkXMLFileReadTester.h"

#include <sys/stat.h>

vtkCxxRevisionMacro(vtkXMLReader, "1.2");

//----------------------------------------------------------------------------
vtkXMLReader::vtkXMLReader()
{
  this->FileName = 0;
  this->FileStream = 0;
  this->XMLParser = 0;  
}

//----------------------------------------------------------------------------
vtkXMLReader::~vtkXMLReader()
{
  this->SetFileName(0);
  if(this->XMLParser)
    {
    this->DestroyXMLParser();
    }
}

//----------------------------------------------------------------------------
void vtkXMLReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " <<
    (this->FileName? this->FileName:"(none)") << "\n";
}

//----------------------------------------------------------------------------
vtkDataSet* vtkXMLReader::GetOutputAsDataSet()
{
  if(this->NumberOfOutputs < 1) { return 0; }
  return static_cast<vtkDataSet*>(this->Outputs[0]);
}

//----------------------------------------------------------------------------
int vtkXMLReader::CanReadFileVersion(int, int)
{
  return 1;
}

//----------------------------------------------------------------------------
int vtkXMLReader::OpenVTKFile()
{
  if(this->FileStream)
    {
    vtkErrorMacro("File already open.");
    return 1;
    }
  this->FileStream = new ifstream;
  this->FileStream->open(this->FileName);
  
  if(!(*this->FileStream))
    {
    vtkErrorMacro("Error opening file " << this->FileName);
    delete this->FileStream;
    this->FileStream = 0;
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkXMLReader::CloseVTKFile()
{
  if(!this->FileStream)
    {
    vtkErrorMacro("File not open.");
    return;
    }
  this->FileStream->close();
  delete this->FileStream;
  this->FileStream = 0;
}

//----------------------------------------------------------------------------
void vtkXMLReader::CreateXMLParser()
{
  if(this->XMLParser)
    {
    vtkErrorMacro("CreateXMLParser() called with existing XMLParser.");
    this->DestroyXMLParser();
    }
  this->XMLParser = vtkXMLDataParser::New();
}

//----------------------------------------------------------------------------
void vtkXMLReader::DestroyXMLParser()
{
  if(!this->XMLParser)
    {
    vtkErrorMacro("DestroyXMLParser() called with no current XMLParser.");
    return;
    }
  this->XMLParser->Delete();
  this->XMLParser = 0;
}

//----------------------------------------------------------------------------
void vtkXMLReader::SetupCompressor(const char* type)
{
  // Instantiate a compressor of the given type.
  if(!type)
    {
    vtkErrorMacro("Compressor has no type.");
    return;
    }
  vtkObject* object = vtkInstantiator::CreateInstance(type);
  vtkDataCompressor* compressor = vtkDataCompressor::SafeDownCast(object);
  if(!compressor)
    {
    vtkErrorMacro("Error creating " << type);
    if(object) { object->Delete(); }
    return;
    }
  this->XMLParser->SetCompressor(compressor);
  compressor->Delete();
}

//----------------------------------------------------------------------------
void vtkXMLReader::ExecuteInformation()
{
  // Destroy any old information that was parsed.
  if(this->XMLParser)
    {
    this->DestroyXMLParser();
    }
  
  // Open the input file.  If it fails, the error was already
  // reported by OpenVTKFile.
  if(!this->OpenVTKFile())
    {
    return;
    }
  
  // Create the vtkXMLParser instance used to parse the file.
  this->CreateXMLParser();
  
  // Configure the parser for this file.
  this->XMLParser->SetStream(this->FileStream);
  
  // Parse the input file.
  if(this->XMLParser->Parse())
    {
    // Let the subclasses read the information they want.
    this->ReadXMLInformation();
    }
  else
    {
    vtkErrorMacro("Error parsing input file.  ExecuteInformation aborting.");
    }
  
  // Close the file to prevent resource leaks.
  this->CloseVTKFile();
}

//----------------------------------------------------------------------------
void vtkXMLReader::ExecuteData(vtkDataObject* vtkNotUsed(output))
{
  // Re-open the input file.  If it fails, the error was already
  // reported by OpenVTKFile.
  if(!this->OpenVTKFile())
    {
    return;
    }
  if(!this->XMLParser)
    {
    vtkErrorMacro("ExecuteData called with no current XMLParser.");
    }
  
  // Give the vtkXMLParser instance its file back so that data section
  // reads will work.
  this->XMLParser->SetStream(this->FileStream);
  
  // Let the subclasses read the data they want.
  this->ReadXMLData();
  
  // Close the file to prevent resource leaks.
  this->CloseVTKFile();
}

//----------------------------------------------------------------------------
void vtkXMLReader::ReadXMLInformation()
{
  // Read from the representation of the XML file.
  this->ReadVTKFile(this->XMLParser->GetRootElement());
  
  // Setup the reader's output with information from the file.
  this->SetupOutputInformation();
}

//----------------------------------------------------------------------------
void vtkXMLReader::ReadXMLData()
{
  // Allocate the output's data.
  this->SetupOutputData();
}

//----------------------------------------------------------------------------
int vtkXMLReader::ReadVTKFile(vtkXMLDataElement* eVTKFile)
{
  // Setup the compressor if there is one.
  const char* compressor = eVTKFile->GetAttribute("compressor");
  if(compressor) { this->SetupCompressor(compressor); }
  
  // Get the primary element.
  const char* name = this->GetDataSetName();
  int i;
  vtkXMLDataElement* ePrimary = 0;
  for(i=0; i < eVTKFile->GetNumberOfNestedElements(); ++i)
    {
    vtkXMLDataElement* eNested = eVTKFile->GetNestedElement(i);
    if(strcmp(eNested->GetName(), name) == 0)
      {
      ePrimary = eNested;
      break;
      }
    }
  if(!ePrimary)
    {
    vtkErrorMacro("Cannot find " << name << " element in file.");
    return 0;
    }
  
  // Read the primary element.
  return this->ReadPrimaryElement(ePrimary);
}

//----------------------------------------------------------------------------
int vtkXMLReader::ReadPrimaryElement(vtkXMLDataElement*)
{
  // We don't need any information from the primary element here.
  return 1;
}

//----------------------------------------------------------------------------
void vtkXMLReader::SetupOutputInformation()
{
  // Initialize the output.
  this->GetOutputAsDataSet()->Initialize();  
}

//----------------------------------------------------------------------------
void vtkXMLReader::SetupOutputData()
{
  // Setup information first.
  this->SetupOutputInformation();
}

//----------------------------------------------------------------------------
vtkDataArray* vtkXMLReader::CreateDataArray(vtkXMLDataElement* da)
{
  int dataType = 0;
  if(!da->GetWordTypeAttribute("type", dataType))
    {
    return 0;
    }  
  
  vtkDataArray* array = vtkDataArray::CreateDataArray(dataType);
  
  array->SetName(da->GetAttribute("Name"));
  
  int components;
  if(da->GetScalarAttribute("NumberOfComponents", components))
    {
    array->SetNumberOfComponents(components);
    }
  
  return array;
}

//----------------------------------------------------------------------------
int vtkXMLReader::CanReadFile(const char* name)
{
  // First make sure the file exists.  This prevents an empty file
  // from being created on older compilers.
  struct stat fs;
  if(stat(name, &fs) != 0) { return 0; }
  
  // Test if the file with the given name is a VTKFile with the given
  // type.
  vtkXMLFileReadTester* tester = vtkXMLFileReadTester::New();
  tester->SetFileName(name);
  
  int result = 0;
  if(tester->TestReadFile() && tester->GetFileDataType())
    {
    if(strcmp(tester->GetFileDataType(), this->GetDataSetName()) == 0)
      {
      const char* version = tester->GetFileVersion();
      if(version)
        {
        if(this->CanReadFileVersionString(version))
          {
          result = 1;
          }
        }
      else
        {
        result = 1;
        }
      }
    }
  
  tester->Delete();
  return result;
}

//----------------------------------------------------------------------------
int vtkXMLReader::CanReadFileVersionString(const char* version)
{
  // Extract the major and minor version numbers.
  int length = strlen(version);
  int major = 0;
  int minor = 0;
  const char* begin = version;
  const char* end = version + length;
  const char* s;
  
  for(s=begin; (s != end) && (*s != '.'); ++s);
  
  if(s > begin)
    {
    strstream str;
    str.write(begin, s-begin);
    str << ends;
    str >> major;
    if(!str)
      {
      major = 0;
      }
    }
  if(++s < end)
    {
    strstream str;
    str.write(s, end-s);
    str << ends;
    str >> minor;
    if(!str)
      {
      minor = 0;
      }
    }
  
  return this->CanReadFileVersion(major, minor);
}

//----------------------------------------------------------------------------
int vtkXMLReader::IntersectExtents(int* extent1, int* extent2, int* result)
{
  if((extent1[0] > extent2[1]) || (extent1[2] > extent2[3]) ||
     (extent1[4] > extent2[5]) || (extent1[1] < extent2[0]) ||
     (extent1[3] < extent2[2]) || (extent1[5] < extent2[4]))
    {
    // No intersection of extents.
    return 0;
    }
  
  // Get the intersection of the extents.
  result[0] = this->Max(extent1[0], extent2[0]);
  result[1] = this->Min(extent1[1], extent2[1]);
  result[2] = this->Max(extent1[2], extent2[2]);
  result[3] = this->Min(extent1[3], extent2[3]);
  result[4] = this->Max(extent1[4], extent2[4]);
  result[5] = this->Min(extent1[5], extent2[5]);
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkXMLReader::Min(int a, int b)
{
  return (a < b)? a : b;
}

//----------------------------------------------------------------------------
int vtkXMLReader::Max(int a, int b)
{
  return (a > b)? a : b;
}

//----------------------------------------------------------------------------
void vtkXMLReader::ComputeDimensions(int* extent, int* dimensions, int isPoint)
{
  dimensions[0] = extent[1] - extent[0] + isPoint;
  dimensions[1] = extent[3] - extent[2] + isPoint;
  dimensions[2] = extent[5] - extent[4] + isPoint;
}

//----------------------------------------------------------------------------
void vtkXMLReader::ComputeIncrements(int* extent, int* increments, int isPoint)
{
  int dimensions[3];
  this->ComputeDimensions(extent, dimensions, isPoint);
  increments[0] = 1;
  increments[1] = increments[0]*dimensions[0];
  increments[2] = increments[1]*dimensions[1];
}

//----------------------------------------------------------------------------
unsigned int vtkXMLReader::GetStartTuple(int* extent, int* increments,
                                         int i, int j, int k)
{
  unsigned int offset = (i - extent[0]) * increments[0];
  offset += (j - extent[2]) * increments[1];
  offset += (k - extent[4]) * increments[2];
  return offset;
}
