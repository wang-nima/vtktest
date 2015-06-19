#include <vtkPolyData.h>
#include <vtkSTLReader.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkConeSource.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>

#include <vtkPolyData.h>
#include <vtkSTLWriter.h>
#include <vtkSTLReader.h>
#include <vtkSphereSource.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>


#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>      // std::ifstream
#include <cfloat>
#include <cstring>
#include <cmath>

#include <vtkMath.h>

using namespace std;


class v3
{
public:
	v3() {
		x = y = z = 0;
	}
    v3(char* bin);
    float x, y, z;
	v3 const operator + (const v3 &new_vector) const {
		v3 ret;
		ret.x = x + new_vector.x;
		ret.y = y + new_vector.y;
		ret.z = z + new_vector.z;
		return ret;
	}
};

v3::v3(char* facet)
{
    char f1[4] = {facet[0],facet[1],facet[2],facet[3]};
    char f2[4] = {facet[4],facet[5],facet[6],facet[7]};
    char f3[4] = {facet[8],facet[9],facet[10],facet[11]};
 
    x = *((float*) f1 );
    y = *((float*) f2 );
    z = *((float*) f3 );
}


class tri
{
public:
	tri() {}
	tri(v3 normal, v3 p1, v3 p2, v3 p3) {
		this->normal = normal;
		this->p1 = p1;
		this->p2 = p2;
		this->p3 = p3;
	}
	tri const operator + (const v3 &translation_vector) const {
		tri ret;
		ret.normal = normal;
		ret.p1 = p1 + translation_vector;
		ret.p2 = p2 + translation_vector;
		ret.p3 = p3 + translation_vector;
		return ret;
	}
    v3 normal, p1, p2, p3;
};


void read_stl(const string &fname, vector<tri> &v, float &min_x, float &max_x, float &min_y, float &max_y){
 
    //!!
    //don't forget ios::binary
    //!!
    ifstream myFile (fname.c_str(), ios::in | ios::binary);
 
    char header_info[80] = "";
    char nTri[4];
    unsigned int nTriLong;
 
    //read 80 byte header
    if (myFile) {
        myFile.read (header_info, 80);
        cout <<"header: " << header_info << endl;
    }
    else{
        cout << "error" << endl;
    }
 
    //read 4-byte ulong
    if (myFile) {
        myFile.read (nTri, 4);
        nTriLong = *((unsigned int*)nTri) ;
        cout <<"n Tri: " << nTriLong << endl;
		v.reserve(nTriLong);
    }
    else{
        cout << "error" << endl;
    }
 
    //now read in all the triangles
    for(int i = 0; i < nTriLong; i++){
 
        char facet[50];
 
        if (myFile) {
 
        	// read one 50-byte triangle
            myFile.read (facet, 50);
 
			v3 normal(facet);
            v3 p1(facet+12);
            v3 p2(facet+24);
            v3 p3(facet+36);

			float x1 = p1.x;
			float x2 = p2.x;
			float x3 = p3.x;
			float y1 = p1.y;
			float y2 = p2.y;
			float y3 = p3.y;

			min_x = min(min_x, min(min(x1, x2), x3));
			min_y = min(min_y, min(min(y1, y2), y3));
			max_x = max(max_x, max(max(x1, x2), x3));
			max_y = max(max_y, max(max(y1, y2), y3));

            // add a new triangle to the array
            v.push_back( tri(normal,p1,p2,p3) );
        }
    }
    return;
}

void save_stl_file(const vector<tri> &v) {
	ofstream f("out.stl", ios::out | ios::binary);

	char header[80];
	memset(header, 0, sizeof(header));
	f.write(header, 80);
	int size = v.size();
	f.write((char*)&size, sizeof(int));

	char empty[2];
	memset(empty, 0, sizeof(empty));

	// 50 byte for one surface
	for (int i = 0; i < v.size(); i++) {
		tri temp = v[i];
		f.write((char*)&temp, sizeof(tri));
		f.write(empty, sizeof(empty));
	}

}

 
int main ( int argc, char *argv[] )
{
	if ( argc != 2 ) {
		cout << "Required parameters: Filename" << endl;
		return EXIT_FAILURE;
    }
	std::string inputFilename = argv[1];

	string file_name(argv[1]);
	vector<tri> v;
	float min_x = FLT_MAX;
	float max_x = FLT_MIN;
	float min_y = FLT_MAX;
	float max_y = FLT_MIN;
	read_stl(file_name, v, min_x, max_x, min_y, max_y);
 
	vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName(inputFilename.c_str());
	reader->Update();
 
	vtkPolyData *stlPolyData =vtkPolyData::New(); 
	stlPolyData = reader->GetOutput(); 


	float down[3];
	down[0] = 0;
	down[1] = 0;
	down[2] = -1;

	const float PI = 3.14159265;

	float min_angle = 180.0;
	tri min_angle_surface;

	// compute which normal vector is closest to (0, 0, -1)
	for (int i = 0; i < v.size(); i++) {
		v3 temp = v[i].normal;
		//cout << temp.x << " " << temp.y << " " << temp.z << endl;
		float x[3];
		x[0] = temp.x;
		x[1] = temp.y;
		x[2] = temp.z;
		float result = vtkMath::Dot(x, down);			// both normal(x) and down are length 1
		float angle = result = acos (result) * 180.0 / PI;
		if (angle < min_angle) {
			min_angle = angle;
			min_angle_surface = v[i];
		}
	}
	v3 min_angle_surface_normal = min_angle_surface.normal;
	float that_normal[3];
	that_normal[0] = min_angle_surface_normal.x;
	that_normal[1] = min_angle_surface_normal.y;
	that_normal[2] = min_angle_surface_normal.z;

	for (int i = 0; i < 3; i++) {
		cout << that_normal[0] <<" "<< that_normal[1] <<" "<< that_normal[2] << endl;
	}

	float rotate_axis[3];
	vtkMath::Cross(that_normal, down, rotate_axis);
	cout << min_angle << endl;
	for (int i = 0; i < 3; i++) {
		cout << rotate_axis[0] <<" "<< rotate_axis[1] <<" "<< rotate_axis[2] << endl;
	}
	cout << vtkMath::Normalize(rotate_axis) << endl;
	for (int i = 0; i < 3; i++) {
		cout << rotate_axis[0] <<" "<< rotate_axis[1] <<" "<< rotate_axis[2] << endl;
	}

	//cout << stlPolyData->GetNumberOfCells() << endl;
	//cout << stlPolyData->GetNumberOfPoints() << endl;
	//stlPolyData->GetCellPoints();
	//vtkCellArray *stlCellArray = stlPolyData->GetPolys();
	//std::cout << "There are " << sphereSource->GetOutput()->GetNumberOfCells() << " cells." << std::endl;

	//int nCellsNum = stlPolyData->GetNumberOfCells(); 

	//cout << nCellsNum << endl;

	//for (int nCellID = 0; nCellID < nCellsNum; nCellID++) 
	//{ 
	//	cout << "face " << nCellID << endl;
	//	vtkCell* cell; 
	//	cell = stlPolyData->GetCell(nCellID); 
	//	vtkPoints *Points = vtkPoints::New(); 
	//	Points = cell->GetPoints(); 
	//	             
	//	vtkIdList *PointsIDList =vtkIdList::New(); 
	//	 //	 stlPolyData->GetCellPoints(nCellID,PointsIDList); 
	//	PointsIDList=cell->GetPointIds();             
	//	int a = 0; 
	//	int b = 0; 
	//	int c = 0; 

	//	double* dpPointA; 
	//	double* dpPointB; 
	//	double* dpPointC;

	//	double x1,y1,z1,x2,y2,z2,x3,y3,z3; 

	//	a = PointsIDList->GetId(0); 
	//	dpPointA = Points->GetPoint(a); 
	//	x1=dpPointA[0]; 
	//	y1=dpPointA[1]; 
	//	z1=dpPointA[2]; 

	//	b = PointsIDList->GetId(1); 
	//	dpPointB = Points->GetPoint(b); 
	//	x2=dpPointB[0]; 
	//	y2=dpPointB[1]; 
	//	z2=dpPointB[2]; 

	//	c = PointsIDList->GetId(2); 
	//	dpPointC = Points->GetPoint(c); 
	//	x3=dpPointC[0]; 
	//	y3=dpPointC[1]; 
	//	z3=dpPointC[2]; 

	//	cout << x1 << " " << y1 << " " << z1 << endl;
	//	cout << x2 << " " << y2 << " " << z2 << endl;
	//	cout << x3 << " " << y3 << " " << z3 << endl;


	//} 

	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->RotateWXYZ(min_angle, rotate_axis[0], rotate_axis[1], rotate_axis[2]);
	
	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
	    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	
	transformFilter->SetTransform(transform);
	transformFilter->SetInputConnection(reader->GetOutputPort());
	transformFilter->Update();

	vtkSmartPointer<vtkSTLWriter> stlWriter =
    vtkSmartPointer<vtkSTLWriter>::New();
	stlWriter->SetFileName("hehe.stl");
	stlWriter->SetInputConnection(transformFilter->GetOutputPort());
	stlWriter->Write();



  // Visualize
  //vtkSmartPointer<vtkPolyDataMapper> mapper =
  //  vtkSmartPointer<vtkPolyDataMapper>::New();
  //mapper->SetInputConnection(reader->GetOutputPort());
 
  //vtkSmartPointer<vtkActor> actor =
  //  vtkSmartPointer<vtkActor>::New();
  //actor->SetMapper(mapper);
 
  //vtkSmartPointer<vtkRenderer> renderer =
  //  vtkSmartPointer<vtkRenderer>::New();
  //vtkSmartPointer<vtkRenderWindow> renderWindow =
  //  vtkSmartPointer<vtkRenderWindow>::New();
  //renderWindow->AddRenderer(renderer);
  //vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
  //  vtkSmartPointer<vtkRenderWindowInteractor>::New();
  //renderWindowInteractor->SetRenderWindow(renderWindow);
 
  //renderer->AddActor(actor);
  //renderer->SetBackground(.3, .6, .3); // Background color green
 
  //renderWindow->Render();
  //renderWindowInteractor->Start();
 
  return EXIT_SUCCESS;
}

