//
//  dazToPLY.h
//  yaluxplug
//
//  Created by Daniel Bui on 5/3/15.
//
//

#ifndef yaluxplug_dazToPLY_h
#define yaluxplug_dazToPLY_h

// just declarations here, definitions at bottom
class PLY_VertexElement;
class PLY_FaceElement;

//////////
// DazToPLY class
//      Main class for exporting Daz Mesh/UV data to Luxrender compliant PLY
///////////
class DazToPLY
{
public:
    DazToPLY(DzFacetMesh *mesh, QString meshName, DzMaterial *mat);
    void walkFaceEdges(DzFacet *face);
    void processFace(DzFacet *face);
    QString LuxMakeAsciiPLY();    
    QString LuxMakeBinPLY();
    void cachedAddDazVertexIndex(DzFacet *face, int daz_vertex_index, int *ply_current_vertex_index, int icurrentVert);
    bool alreadyProcessed(int daz_vertex_index, int *daz_cached_index);

public:
    DzFacetMesh *mesh;
    DzMaterialFaceGroup *facetIndexList;
    QString objMatName;
    
    DzFacet *ptrAllFaces;
    DzEdge  *ptrAllEdges;
    DzPnt3  *ptrAllVertices;
    DzPnt3  *ptrAllNormals;
    DzMap   *ptrAllUVs;
    unsigned char *ptrAllFacetFlags;

    QList<PLY_VertexElement> ply_vertexElList;
    QList<PLY_FaceElement> ply_faceElList;
    QList<int> ply_dazVertexIndexLookupTable;
    QBitArray daz_indexIsCached;
    
};

class PLY_VertexElement
{
public:
    PLY_VertexElement() {};
    PLY_VertexElement(const PLY_VertexElement &old) {
        x = old.x;
        y = old.y;
        z = old.z;
        nx = old.nx;
        ny = old.ny;
        nz = old.nz;
        s = old.s;
        t = old.t;
    };
    QString getString();
    char* getByteArray();
    static int sizeofByteArray() { return sizeof(float[8]); };
    int lookupDazIndex(int daz_vertex_index);
    void cached_AddDazVertexIndex();
    
public:
    float x;
    float y;
    float z;
    float nx;
    float ny;
    float nz;
    float s;
    float t;
    
private:
    int cached_DazVertexIndex =-1;
    float data[8];
    
};

class PLY_FaceElement
{
public:
    QString getString();
    char* getByteArray();
    int sizeofByteArray() {
        int len = sizeof(uchar);
        len += sizeof(uint32_t)*numV;
        
        return len;
    };
    
public:
    int numV;
    int v[4] = {-1,-1,-1,-1};

private:
    char data[17];
    
};


#endif // yaluxplug_dazToPLY_h
