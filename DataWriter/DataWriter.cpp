// EmbedData.cpp : Defines the entry point for the console application.
//

#ifdef LINUX
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <vector>

	#include <3DIO/3DIO.h>
	#define DATA_SIZE 2000 
	typedef struct footer {
		size_t oSize;
		char sig[5];
	} Footer;

	int main(int argc, char *argv[])
	{
		float data[10];
		for(int i = 0; i < 10; i++)data[i] = i;

		Footer foot;
		FILE *stream = fopen("embeddata2.exe","ab");
		
		if(!stream){
			printf("  - Couldn't open embeddata.exe for append.\n");
			return -1;
		}
		///Get file size
		fseek(stream, 0L, SEEK_END);
		foot.oSize = ftell(stream);
		sprintf(foot.sig,"eb1.0");

		//Return to head
		//fseek(fp, 0L, SEEK_SET);
		
		fwrite(data,sizeof(double),10,stream);
		fwrite(&foot,sizeof(Footer),1,stream);

		fclose(stream);

		return 0;
	}

#else
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <vector>

	#include <3DIO/3DIO.h>
	#define DATA_SIZE 2000 
	
	using namespace tdio_library;
	bool write_vertices(HANDLE hResource,Object<PLY> &ply);
	bool write_color(HANDLE hResource,Object<PLY> &ply);
	bool read_obj(const char *file, float **data, int &size);


	int main(int argc, char *argv[])
	{
		HANDLE hResource = BeginUpdateResource(L".\\EmbedData.exe",FALSE);
	 
		float test_data[DATA_SIZE];
		for(int i = 0; i < DATA_SIZE; i++){
			test_data[i] = i;
		}
		if(hResource==NULL){
			DWORD err = GetLastError();;

			printf("  - Error[%d], couldn't open handle to EmbedData.exe\n",err);
			return 0;
		} else {
			Object<PLY> ply;
			tdio_library::GDebugger::Instance().SetVerbosity(5);
			
			if(argc == 1){
				if(!Reader::ReadPLY("test.ply",ply)){
					printf("  - Error, problem w/ file\n");
					return 0;
				}
			} else {
				if(!Reader::ReadPLY(argv[1],ply)){
					printf("  - Error, problem w/ file\n");
					return 0;
				}
			}
			if(!write_vertices(hResource,ply))return 0;
			if(!write_color(hResource,ply)){
				printf("  - Failed to write color.\n");
			}
			
			///End resource updating
			EndUpdateResource(hResource, FALSE);
			
		}

		return 0;
	}
	bool write_vertices(HANDLE hResource,Object<PLY> &ply){
		vector3 *verts = ply.GetVerticesAsVectors();
		int nVtx = ply.GetNumVertices();

		printf("- Writing vertices\n");
		printf("  - nVtx: %d\n",nVtx);
		int size = (nVtx * 3) + 1;
		float *data = new float[size];
		data[0] = size-1;
		int d = 0;
		for(int v = 0; v < nVtx; v++){
			data[d+1] = verts[v].x;
			data[d+2] = verts[v].y;
			data[d+3] = verts[v].z;
			d+=3;
		}

		printf("  - d: %d  which is %d verts\n",d,d/3);
		if (UpdateResource(hResource, 
			RT_RCDATA, 
			MAKEINTRESOURCE(100), 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPVOID) data, 
			(sizeof(float)*size)) != FALSE)
		{
			printf("  - Successfully wrote %d bytes(%d vertices) to resource.\n",sizeof(float)*size,size);

		} else {
			printf("  - Failed to write data to vertex resource\n");
			return false;
		}
		if(data){delete [] data; data = NULL;}

		return true;
	}

	bool write_color(HANDLE hResource,Object<PLY> &ply){
		rgb_l *colors = ply.GetColors();
		if(!colors)return false;

		int nVtx = ply.GetNumVertices();

		printf("- Writing colors\n");
		printf("  - nColors: %d\n",nVtx);
		int size = (nVtx * 3) + 1;
		float *data = new float[size];
		data[0] = size-1;
		int d = 0;
		for(int v = 0; v < nVtx; v++){
			if(v < 10){
				printf("color[%d]: %f %f %f\n",v,colors[v].r,colors[v].g,colors[v].b);
			}
			data[d+1] = colors[v].r;
			data[d+2] = colors[v].g;
			data[d+3] = colors[v].b;
			d+=3;
		}

		if (UpdateResource(hResource, 
			RT_RCDATA, 
			MAKEINTRESOURCE(101), 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPVOID) data, 
			(sizeof(float)*size)) != FALSE)
		{
			printf("  - Successfully wrote %d bytes(%d vertices) to resource.\n",sizeof(float)*size,size);
			EndUpdateResource(hResource, FALSE);
		} else {
			printf("  - Failed to write data to color resource\n");
			return false;
		}
		if(data){delete [] data; data = NULL;}
		return true;
	}
	bool read_obj(const char *file, float **data, int &size){
		FILE *stream = fopen(file,"r");
		if(!stream){
			printf("  - ERROR: Couldn't open %s for reading\n",file);
			return false;
		}
		std::vector<float> v;
		char buffer[256];
		while(!feof(stream)){
			if(fgets(buffer,256,stream)==NULL)break;
			float x,y,z;
			char ch;
			if(buffer[0] == 'v'){
				sscanf(buffer,"%c %f %f %f\n",&ch,&x,&y,&z);
				v.push_back(x);v.push_back(y);v.push_back(z);
			}
		}
		printf("Read %d vertices from obj file\n",v.size());
		*data = new float[v.size()+1];
		(*data)[0] = v.size();
		for(int i = 0; i < v.size(); i++){
			(*data)[i+1] = v[i];
		}
		size = v.size();
		fclose(stream);
		return true;
	}
#endif