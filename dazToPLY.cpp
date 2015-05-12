//
//  dazToPLY.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 5/5/15.
//  Copyright (c) 2015 Daniel Bui. All rights reserved.
//

#include "dzfileio.h"
#include "dztexture.h"
#include "dzproperty.h"
#include "dzobject.h"
#include "dzshape.h"
#include "dzgeometry.h"
#include "dzmaterial.h"
#include "dzdefaultmaterial.h"
#include "dztarray.h"
#include "dzstringproperty.h"
#include "dzcolorproperty.h"
#include "dzimageproperty.h"
#include "dzfloatproperty.h"
#include "dzintproperty.h"
#include "dznodeproperty.h"
#include "dznumericproperty.h"
#include "dzvertexmesh.h"
#include "dzfacetmesh.h"
#include "dzfacegroup.h"

#include "dazToPLY.h"
#include "renderer.h"
#include "plugin.h"

QString PLY_FaceElement::getString()
{
    QString buffer;
    buffer = QString("%1 ").arg(numV);
    int i = 0;
    while (i < numV)
    {
        buffer += QString("%1 ").arg(v[i]);
        i++;
    }
    buffer += "\n";
    
    return buffer;
    
}

char* PLY_FaceElement::getByteArray()
{
    //char *byteArray = new char[sizeofByteArray()];
    uint32_t *ptr;
    
    data[0] = (char) numV;
    
    ptr = (uint32_t*) (&data[1]);
    
    int i=0;
    while (i < numV)
    {
        ptr[i] = v[i];
        i++;
    }
    
    return data;
}

QString PLY_VertexElement::getString()
{
    QString buffer;
    buffer = QString("%1 %2 %3 %4 %5 %6 %7 %8\n").arg(x).arg(y).arg(z).arg(nx).arg(ny).arg(nz).arg(s).arg(t);
    
    return buffer;
}

char* PLY_VertexElement::getByteArray()
{
    
    data[0] = x;
    data[1] = y;
    data[2] = z;
    data[3] = nx;
    data[4] = ny;
    data[5] = nz;
    data[6] = s;
    data[7] = t;
    
    return (char*) data;
}

bool DazToPLY::alreadyProcessed(int daz_vertex_index, int *daz_used_index)
{
    if ( (daz_used_index[0] == daz_vertex_index) || (daz_used_index[1] == daz_vertex_index) ||
        (daz_used_index[2] == daz_vertex_index) || (daz_used_index[3] == daz_vertex_index) )
    {
        // this vertex index has already been added
        return true;
    }
    else
        return false;
}

/// for use by walkQuadEdges
/// create a function to wrap adding the floats and caching the vertex index
void DazToPLY::cachedAddDazVertexIndex(DzFacet *face, int daz_vertex_index, int *ply_current_vertex_index, int icurrentVert)
{
    float *float_ptr;
    
    // look up daz_v_index in 
    if (daz_indexIsCached.testBit(daz_vertex_index) == false)
    {
        // copy to the ply vertex list...
        // grab the ptr to float[3]
        float_ptr = (float*) &ptrAllVertices[daz_vertex_index];
        // add the float[3] array to ply vertex list
        
        PLY_VertexElement vtx_el;
        vtx_el.x = float_ptr[0]/100;
        vtx_el.y = -float_ptr[2]/100;
        vtx_el.z = float_ptr[1]/100;
        
        // Now add the additional PLY vertex data (normals and uv)
        int normals_index = face->m_normIdx[icurrentVert]; // get corresponding i'th index for normals
        float_ptr = (float*) &ptrAllNormals[normals_index];
        vtx_el.nx = float_ptr[0];
        vtx_el.ny = -float_ptr[2];
        vtx_el.nz = float_ptr[1];        
        int uv_index = face->m_uvwIdx[icurrentVert];
        float_ptr = (float*) ptrAllUVs->getPnt2Value(uv_index);
        vtx_el.s = float_ptr[0];
        vtx_el.t = float_ptr[1];         
        
        ply_vertexElList.append( vtx_el );
        ply_dazVertexIndexLookupTable.append(daz_vertex_index); 
        // then cache the daz index to the ply index. NB: always add both together        
        
        // save the ply index so we can add all the vert_indices to the ply face list
        ply_current_vertex_index[icurrentVert] = ply_vertexElList.count()-1;
    }
    else
    {
        // vertex already cached, so lookup the index and save it for writing to ply face list
        ply_current_vertex_index[icurrentVert] = ply_dazVertexIndexLookupTable.indexOf(daz_vertex_index);
    }
    
}


void DazToPLY::processFace( DzFacet *face )
{
    int numVerts;
    int icurrentVert;
    if (face->isQuad())
        numVerts = 4;
    else
        numVerts = 3;
    
    int daz_vertex_index; // index into Daz vertex list
    int ply_vertex_index[4] = {-1,-1,-1,-1 }; // index into PLY vertex list
    int daz_used_index[4] = {-1,-1,-1,-1 };

    PLY_FaceElement faceEl;
    faceEl.numV = numVerts;
    
    icurrentVert = 0; // index of vertex to be added into list
    while (icurrentVert < numVerts) // one iteration for each edge of face
    {
        // get the index to the current edge
        daz_vertex_index = face->m_vertIdx[icurrentVert];
        
        // Compare index to any indices already present in ply vertex index list
        if (!alreadyProcessed(daz_vertex_index, daz_used_index))
        {
            // vertex index not already saved in ply, so do a cached add
            cachedAddDazVertexIndex(face, daz_vertex_index, ply_vertex_index, icurrentVert);
            daz_used_index[icurrentVert] = daz_vertex_index;            
            
            // add vertex to face Element
            faceEl.v[icurrentVert] = ply_vertex_index[icurrentVert];
        }
        icurrentVert++;
    }
    // add face Element to ply list
    ply_faceElList.append( faceEl );
    
}


void DazToPLY::walkFaceEdges(DzFacet *face )
{
    // make a max size boolean lookup table *QBitArray, set it to true when vertex is added to vertex list
    // bitArray done...
    
    // 1. start first edge
    // 1.a vertex = 1
    // if index is new, add it to vertex list
    
    // 1.b vertex = 2
    // if index is new, add it to vertex list
    
    // 2. start second edge
    // 2.a (find new index compared to 1.a & 1.b)
    // search to see if index exists
    // 2.b (new vertex = 3)
    // 3. start third edge
    // 3.a (find new vertex )
    // 3.b (new vertex = 4)
    
    int icurrentVert;
    int edge_index;
    int numEdges;
    if (face->isQuad())
        numEdges = 4;
    else
        numEdges = 3;
    
    int daz_vertex_index; // index into Daz vertex list
    int ply_vertex_index[4] = {-1,-1,-1,-1 }; // index into PLY vertex list
    int daz_used_index[4] = {-1,-1,-1,-1 };

    PLY_FaceElement faceEl;
    faceEl.numV = numEdges;
    
    icurrentVert = 0; // index of vertex to be added into list
    int icurrentEdge = 0;
    while (icurrentEdge < numEdges) // one iteration for each edge of face
    {
        // get the index to the current edge
        edge_index = face->m_edges[icurrentEdge];
        // get the first vertex index
        daz_vertex_index = ptrAllEdges[edge_index].m_vert1;
        
        // Compare index to any indices already present in ply vertex index list
        if (!alreadyProcessed(daz_vertex_index, daz_used_index))
        {
            // vertex index not already saved in ply, so do a cached add
            cachedAddDazVertexIndex(face, daz_vertex_index, ply_vertex_index, icurrentVert);
            daz_used_index[icurrentVert] = daz_vertex_index;            
            
            // add vertex to face Element
            faceEl.v[icurrentVert] = ply_vertex_index[icurrentVert];
            icurrentVert++;
            if (icurrentVert == numEdges)
                break;
        }
        
        // now repeat above for vertex #2 of this edge
        daz_vertex_index = ptrAllEdges[edge_index].m_vert2;
        // Compare index to any indices already present in ply vertex index list
        if (!alreadyProcessed(daz_vertex_index, daz_used_index))
        {
            // vertex index not already saved in ply, so do a cached add
            cachedAddDazVertexIndex(face, daz_vertex_index, ply_vertex_index, icurrentVert);            
            daz_used_index[icurrentVert] = daz_vertex_index;            
            
            // add vertex to face Element
            faceEl.v[icurrentVert] = ply_vertex_index[icurrentVert];
            icurrentVert++;
            if (icurrentVert == numEdges)
                break;
        }
        icurrentEdge++;
    }
    // add face Element to ply list
    ply_faceElList.append( faceEl );
    
    
}


QString DazToPLY::LuxMakeBinPLY()
{
    if (facetIndexList == NULL)
        return "";
    const int *face_indexToMesh = facetIndexList->getIndicesPtr();
    int numFaceIndices = facetIndexList->count();
    
    int numFaces;
    int numVerts;
    QString ret_str ="";
    QString filenamePLY;
    
    
    DzFacet *currentFace;
    
    int i = 0;
    while (i < numFaceIndices)
    {
        currentFace = &ptrAllFaces[face_indexToMesh[i]];
        // add entries for quad
        //walkFaceEdges(currentFace);
        processFace(currentFace);
        i++;
    }
    
    numVerts = ply_vertexElList.count();
    numFaces = ply_faceElList.count();
    
    // create resource directory if doesn't exist
    //    DzFileIO::pathExists(YaLuxGlobal.pathTempName + "-resource", true);
    // open meshName.ply for writing inside resource directory
    //    QString tempname = dzApp->getTempFilename();
    filenamePLY = QString("%1.%2.ply").arg(YaLuxGlobal.tempCounter++).arg(objMatName);
    filenamePLY = DzFileIO::fixName(filenamePLY);
    //DEBUG
    dzApp->log("yaluxplug: filenamePLY = [" + filenamePLY + "]");
    dzApp->log("yaluxplug: YaLuxGlobal.pathTempName = [" + YaLuxGlobal.tempPath + "]");
    filenamePLY = QString("%1/%2_%3").arg(YaLuxGlobal.tempPath).arg(YaLuxGlobal.tempFilenameBase).arg(filenamePLY);
    //    dzApp->log( QString("meshname = [%1]. filename = [%2]\n").arg(objMatName).arg(filenamePLY) );
    dzApp->log("yaluxplug: creating PLY file: " + filenamePLY);
    QFile plyOut(filenamePLY);
    
    plyOut.open(QIODevice::ReadWrite);
    plyOut.write("ply\n");
    plyOut.write("format binary_little_endian 1.0\n");
    plyOut.write("comment PLY generated by yaluxplug\n");
    plyOut.write( QString("element vertex %1\n").arg(numVerts) );
    plyOut.write("property float x\n");
    plyOut.write("property float y\n");
    plyOut.write("property float z\n");
    plyOut.write("property float nx\n");
    plyOut.write("property float ny\n");
    plyOut.write("property float nz\n");
    plyOut.write("property float s\n");
    plyOut.write("property float t\n");
    plyOut.write( QString("element face %1\n").arg(numFaces) );
    plyOut.write("property list uchar uint vertex_indices\n");
    plyOut.write("end_header\n");
    
    i = 0;
    int len = ply_vertexElList[i].sizeofByteArray();
    int ibyteswritten;
    char *bytearray;
    while (i < numVerts)
    {
        // write out the vertices
        bytearray = ply_vertexElList[i].getByteArray();
        ibyteswritten = plyOut.write(bytearray, len );
        if (ibyteswritten != len)
        {
            // DEBUG
            dzApp->log("yaluxplug: ERROR writing vertices in ply bin. byteswritten= " + QString("%1").arg(len) );
        }
        i++;
    }
    
    i = 0;
    while (i < numFaces)
    {
        // write out the faces
        len = ply_faceElList[i].sizeofByteArray();
        bytearray = ply_faceElList[i].getByteArray();
        ibyteswritten = plyOut.write(bytearray, len);
        if (ibyteswritten != len)
        {
            // DEBUG
            dzApp->log("yaluxplug: ERROR writing faces in ply bin. byteswritten= " + QString("%1").arg(len));
        }        i++;
    }
    
    plyOut.close();
    
    return filenamePLY;
}


QString DazToPLY::LuxMakeAsciiPLY()
{
    if (facetIndexList == NULL)
        return "";
    const int *face_indexToMesh = facetIndexList->getIndicesPtr();
    int numFaceIndices = facetIndexList->count();
    
    int numFaces;
    int numVerts;
    QString ret_str ="";
    QString filenamePLY;
    
    
    DzFacet *currentFace;
    
    int i = 0;
    while (i < numFaceIndices)
    {
        currentFace = &ptrAllFaces[face_indexToMesh[i]];
        // add entries for quad
        //walkFaceEdges(currentFace);
        processFace(currentFace);
        i++;
    }
    
    numVerts = ply_vertexElList.count();
    numFaces = ply_faceElList.count();
    
    // create resource directory if doesn't exist
    DzFileIO::pathExists(YaLuxGlobal.tempPath + "/" + YaLuxGlobal.tempFilenameBase + "-resource", true);
    // open meshName.ply for writing inside resource directory
    filenamePLY = QString("$1/%2-resource/%3.ply").arg(YaLuxGlobal.tempPath).arg(YaLuxGlobal.tempFilenameBase).arg(objMatName);
    //    dzApp->log( QString("meshname = [%1]. filename = [%2]\n").arg(objMatName).arg(filenamePLY) );
    QFile plyOut(filenamePLY);
    
    plyOut.open(QIODevice::ReadWrite);
    plyOut.write("ply\n");
    plyOut.write("format ascii 1.0\n");
    plyOut.write("comment PLY generated by yaluxplug\n");
    plyOut.write( QString("element vertex %1\n").arg(numVerts) );
    plyOut.write("property float x\n");
    plyOut.write("property float y\n");
    plyOut.write("property float z\n");
    plyOut.write("property float nx\n");
    plyOut.write("property float ny\n");
    plyOut.write("property float nz\n");
    plyOut.write("property float s\n");
    plyOut.write("property float t\n");
    plyOut.write( QString("element face %1\n").arg(numFaces) );
    plyOut.write("property list uchar uint vertex_indices\n");
    plyOut.write("end_header\n");
    
    i = 0;
    while (i < numVerts)
    {
        // write out the vertices
        plyOut.write(ply_vertexElList[i].getString() );
        i++;
    }
    
    i = 0;
    while (i < numFaces)
    {
        // write out the faces
        plyOut.write(ply_faceElList[i].getString());
        i++;
    }
    
    plyOut.close();
    
    return filenamePLY;
    
}


DazToPLY::DazToPLY(DzFacetMesh *arg_mesh, QString arg_objMatName, DzMaterial *arg_mat)
{
    mesh = arg_mesh;
    objMatName = arg_objMatName;
    daz_indexIsCached = QBitArray( mesh->getNumVertices() );
    
    facetIndexList = NULL;
    // find number of face groups
    // create separate PLY for each group
    facetIndexList = mesh->findMaterialGroup(arg_mat->getLabel());
    if (facetIndexList == NULL)
        facetIndexList = mesh->findMaterialGroup(arg_mat->getAssetId());
    if (facetIndexList == NULL)
        facetIndexList = mesh->findMaterialGroup(arg_mat->getName());
    if (facetIndexList == NULL)
    {
        // make an error call and return
        dzApp->log("yaluxplug: LukeMakePLY - ERROR, no facetIndex found");
    }
    
    ptrAllFaces = mesh->getFacetsPtr();
    ptrAllEdges = mesh->getEdgesPtr();
    ptrAllUVs = mesh->getUVs();
    ptrAllNormals = mesh->getNormalsPtr();
    ptrAllVertices = mesh->getVerticesPtr();
    
}
