//
//  ReadyKitRenderer.h
//  ReadyKitRenderer
//
//  Created by Daniel Bui on 5/16/15.
//  Copyright (c) 2015 Daniel Bui. All rights reserved.
//

#ifndef ReadyKitRenderer_h
#define ReadyKitRenderer_h


#include "plugin.h"
#include "dzrenderer.h"
#include "dzrenderdata.h"
#include "dzapp.h"
#include <QtCore/QStringList>
#include <QtCore/QProcess>

class ReadyKitRenderer : public DzRenderer {
    Q_OBJECT
public:
    ReadyKitRenderer();

    // ------------------ Basic control flow, scoping, stacks
    virtual DtVoid  DiWorldBegin()  { dzApp->log("ReadyKitRenderer: unimplemented method: DiWorldBegin()"); }
    virtual DtVoid  DiWorldEnd()    { dzApp->log("ReadyKitRenderer: unimplemented method: DiWorldEnd()"); }
    virtual DtContextHandle DiGetContext()  { dzApp->log("ReadyKitRenderer: unimplemented method: DiGetContext()"); }
    virtual DtVoid  DiContext( DtContextHandle handle ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiContext( DtContextHandle handle ) "); }
    virtual DtVoid  DiAttributeBegin()  { dzApp->log("\nReadyKitRenderer: unimplemented method: DiAttributeBegin()"); }
    virtual DtVoid  DiAttributeEnd()    { dzApp->log("ReadyKitRenderer: unimplemented method: DiAttributeEnd()"); }
    virtual DtVoid  DiTransformBegin()  { dzApp->log("ReadyKitRenderer: unimplemented method: DiTransformBegin()"); }
    virtual DtVoid  DiTransformEnd()    { dzApp->log("ReadyKitRenderer: unimplemented method: DiTransformEnd()"); }
    virtual DtVoid  DiSolidBegin( DtToken type )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiSolidBegin( DtToken type )"); }
    virtual DtVoid  DiSolidEnd()    { dzApp->log("ReadyKitRenderer: unimplemented method: DiSolidEnd()"); }
    virtual DtVoid  DiMotionBeginV( DtInt N, DtFloat times[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiMotionBeginV( DtInt N, DtFloat times[] )"); }
    virtual DtVoid  DiMotionEnd()   { dzApp->log("ReadyKitRenderer: unimplemented method: DiMotionEnd() "); }
    virtual DtToken DiDeclare( const char *name, const char *declaration )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiDeclare( const char *name, const char *declaration )"); }
    virtual DtVoid  DiScreenWindow( DtFloat left, DtFloat right, DtFloat bot, DtFloat top ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiScreenWindow( DtFloat left, DtFloat right, DtFloat bot, DtFloat top )"); }
    virtual DtVoid  DiIfBeginV( DtToken expression, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiIfBeginV( DtToken expression, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiElseIfV( DtToken expression, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiElseIfV( DtToken expression, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiElse()    { dzApp->log("ReadyKitRenderer: unimplemented method: DiElse()"); }
    virtual DtVoid  DiIfEnd()   { dzApp->log("ReadyKitRenderer: unimplemented method: DiIfEnd()"); }
    virtual DtVoid  DiErrorHandler( DtErrorHandler handler )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiErrorHandler( DtErrorHandler handler )"); }
    virtual DtVoid  DiSynchronize( DtToken )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiSynchronize( DtToken )"); }


    // ------------------ Attributes
    virtual DtVoid  DiAttributeV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiAttributeV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiBound( DtBound bound )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiBound( DtBound bound )"); }
    virtual DtVoid  DiColor( DtColor Cs )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiColor( DtColor Cs )"); }
    virtual DtVoid  DiDetail( DtBound bound )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiDetail( DtBound bound )"); }
    virtual DtVoid  DiDetailRange( DtFloat minvis, DtFloat lowtran, DtFloat uptran, DtFloat maxvis )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiDetailRange( DtFloat minvis, DtFloat lowtran, DtFloat uptran, DtFloat maxvis )"); }
    virtual DtVoid  DiGeometricApproximation( DtToken type, DtFloat value ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiGeometricApproximation( DtToken type, DtFloat value )"); }
    virtual DtVoid  DiIlluminate( DtLightHandle light, DtBoolean onoff )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiIlluminate( DtLightHandle light, DtBoolean onoff )"); }
    virtual DtVoid  DiMatte( DtBoolean onoff )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiMatte( DtBoolean onoff )"); }
    virtual DtVoid  DiMultiplyShadingRate( DtFloat ratemultiplier ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiMultiplyShadingRate( DtFloat ratemultiplier )"); }
    virtual DtVoid  DiOpacity( DtColor Cs ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiOpacity( DtColor Cs )"); }
    virtual DtVoid  DiOrientation( DtToken orientation )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiOrientation( DtToken orientation )"); }
    virtual DtVoid  DiReverseOrientation()  { dzApp->log("ReadyKitRenderer: unimplemented method: DiReverseOrientation()"); }
    virtual DtVoid  DiShadingInterpolation( DtToken type )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiShadingInterpolation( DtToken type )"); }
    virtual DtVoid  DiShadingRate( DtFloat size )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiShadingRate( DtFloat size )"); }
    virtual DtVoid  DiSides( DtInt nsides ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiSides( DtInt nsides )"); }
    virtual DtVoid  DiTextureCoordinates( DtFloat s1, DtFloat t1, DtFloat s2, DtFloat t2, DtFloat s3, DtFloat t3, DtFloat s4, DtFloat t4 )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiTextureCoordinates( DtFloat s1, DtFloat t1, DtFloat s2, DtFloat t2, DtFloat s3, DtFloat t3, DtFloat s4, DtFloat t4 )"); }


    // ------------------ Shaders
    virtual DtLightHandle   DiAreaLightSourceV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiAreaLightSourceV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiAtmosphereV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiAtmosphereV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiDisplacementV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiDisplacementV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiExteriorV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method:  DiExteriorV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiImagerV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiImagerV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiInteriorV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiInteriorV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtLightHandle   DiLightSourceV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiLightSourceV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiShaderV( DtToken name, DtToken handle, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiShaderV( DtToken name, DtToken handle, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiSurfaceV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiSurfaceV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }


    // ------------------ Transformations
    virtual DtVoid  DiConcatTransform( DtMatrix transform ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiConcatTransform( DtMatrix transform )"); }
    virtual DtVoid  DiCoordinateSystem( DtToken space ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiCoordinateSystem( DtToken space )"); }
    virtual DtVoid  DiCoordSysTransform( DtToken space )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiCoordSysTransform( DtToken space )"); }
    virtual DtVoid  DiIdentity()    { dzApp->log("ReadyKitRenderer: unimplemented method: DiIdentity()"); }
    virtual DtVoid  DiPerspective( DtFloat fov )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiPerspective( DtFloat fov )"); }
    virtual DtVoid  DiRotate( DtFloat angle, DtFloat dx, DtFloat dy, DtFloat dz )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiRotate( DtFloat angle, DtFloat dx, DtFloat dy, DtFloat dz )"); }
    virtual DtVoid  DiScale( DtFloat dx, DtFloat dy, DtFloat dz )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiScale( DtFloat dx, DtFloat dy, DtFloat dz )"); }
    virtual DtVoid  DiScopedCoordinateSystem( DtToken space )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiScopedCoordinateSystem( DtToken space )"); }
    virtual DtVoid  DiSkew( DtFloat angle, DtFloat dx1, DtFloat dy1, DtFloat dz1, DtFloat dx2, DtFloat dy2, DtFloat dz2 )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiSkew( DtFloat angle, DtFloat dx1, DtFloat dy1, DtFloat dz1, DtFloat dx2, DtFloat dy2, DtFloat dz2 )"); }
    virtual DtVoid  DiTransform( DtMatrix transform )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiTransform( DtMatrix transform )"); }
    virtual DtPoint*    DiTransformPoints( DtToken fromspace, DtToken tospace, DtInt npoints, DtPoint *points ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiTransformPoints( DtToken fromspace, DtToken tospace, DtInt npoints, DtPoint *points )"); }
    virtual DtVoid  DiTranslate( DtFloat dx, DtFloat dy, DtFloat dz )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiTranslate( DtFloat dx, DtFloat dy, DtFloat dz )"); }


    // ------------------ Geometric Primitives
    virtual DtVoid  DiBasis( DtBasis ubasis, DtInt ustep, DtBasis vbasis, DtInt vstep ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiBasis( DtBasis ubasis, DtInt ustep, DtBasis vbasis, DtInt vstep )"); }
    virtual DtVoid  DiBlobbyV( DtInt nleaf, DtInt ncode, DtInt code[], DtInt nflt, DtFloat flt[], DtInt nstr, DtString str[], DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiBlobbyV( DtInt nleaf, DtInt ncode, DtInt code[], DtInt nflt, DtFloat flt[], DtInt nstr, DtString str[], DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiConeV( DtFloat height, DtFloat radius, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiConeV( DtFloat height, DtFloat radius, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiCurvesV( DtToken degree, DtInt ncurves, DtInt nverts[], DtToken wrap, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiCurvesV( DtToken degree, DtInt ncurves, DtInt nverts[], DtToken wrap, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiCylinderV( DtFloat radius, DtFloat zmin, DtFloat zmax, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiCylinderV( DtFloat radius, DtFloat zmin, DtFloat zmax, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiDiskV( DtFloat height, DtFloat radius, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiDiskV( DtFloat height, DtFloat radius, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiGeneralPolygonV (DtInt nloops, DtInt *nverts, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiGeneralPolygonV (DtInt nloops, DtInt *nverts, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiGeometryV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiGeometryV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiHyperboloidV( DtPoint point1, DtPoint point2, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiHyperboloidV( DtPoint point1, DtPoint point2, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiNuCurvesV( DtInt ncurves, DtInt nvertices[], DtInt order[], DtFloat knot[], DtFloat min[], DtFloat max[], DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiNuCurvesV( DtInt ncurves, DtInt nvertices[], DtInt order[], DtFloat knot[], DtFloat min[], DtFloat max[], DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiNuPatchV( DtInt nu, DtInt uorder, DtFloat *uknot, DtFloat umin, DtFloat umax, DtInt nv, DtInt vorder, DtFloat *vknot, DtFloat vmin, DtFloat vmax, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiNuPatchV( DtInt nu, DtInt uorder, DtFloat *uknot, DtFloat umin, DtFloat umax, DtInt nv, DtInt vorder, DtFloat *vknot, DtFloat vmin, DtFloat vmax, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiParaboloidV( DtFloat rmax, DtFloat zmin, DtFloat zmax, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("ReadyKitRenderer: unimplemented method: DiParaboloidV( DtFloat rmax, DtFloat zmin, DtFloat zmax, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiPatchV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiPatchV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiPatchMeshV( DtToken type, DtInt nu, DtToken uwrap, DtInt nv, DtToken vwrap, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiPatchMeshV( DtToken type, DtInt nu, DtToken uwrap, DtInt nv, DtToken vwrap, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiPointsV( DtInt npts, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiPointsV( DtInt npts, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiPointsGeneralPolygonsV( DtInt npolys, DtInt *nloops, DtInt *nverts, DtInt *verts, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiPointsGeneralPolygonsV( DtInt npolys, DtInt *nloops, DtInt *nverts, DtInt *verts, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiPointsPolygonsV( DtInt npolys, DtInt *nverts, DtInt *verts, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiPointsPolygonsV( DtInt npolys, DtInt *nverts, DtInt *verts, DtInt n, const DtToken tokens[], DtPointer params[] )");
    virtual DtVoid  DiPolygonV( DtInt nvertices, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiPolygonV( DtInt nvertices, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiSphereV( DtFloat radius, DtFloat zmin, DtFloat zmax, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiSphereV( DtFloat radius, DtFloat zmin, DtFloat zmax, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiSubdivisionMeshV( DtToken scheme, DtInt nfaces, DtInt nvertices[], DtInt vertices[], DtInt ntags, const DtToken tags[], DtInt nargs[], DtInt intargs[], DtFloat floatargs[], DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiSubdivisionMeshV( DtToken scheme, DtInt nfaces, DtInt nvertices[], DtInt vertices[], DtInt ntags, const DtToken tags[], DtInt nargs[], DtInt intargs[], DtFloat floatargs[], DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiHierarchicalSubdivisionMeshV(	DtToken scheme, DtInt nfaces, DtInt nvertices[], DtInt vertices[], DtInt ntags, const DtToken tags[], DtInt nargs[], DtInt intargs[], DtFloat floatargs[], const DtToken stringargs[], DtInt n, const DtToken tokens[], DtPointer params[]) { dzApp->log("ReadyKitRenderer: unimplemented method: DiHierarchicalSubdivisionMeshV(	DtToken scheme, DtInt nfaces, DtInt nvertices[], DtInt vertices[], DtInt ntags, const DtToken tags[], DtInt nargs[], DtInt intargs[], DtFloat floatargs[], const DtToken stringargs[], DtInt n, const DtToken tokens[], DtPointer params[])"); }
    virtual DtVoid  DiTorusV( DtFloat majorrad, DtFloat minorrad, DtFloat phimin, DtFloat phimax, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiTorusV( DtFloat majorrad, DtFloat minorrad, DtFloat phimin, DtFloat phimax, DtFloat thetamax, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiTrimCurve( DtInt nloops, DtInt *ncurves, DtInt *order, DtFloat *knot, DtFloat *amin, DtFloat *amax, DtInt *n, DtFloat *u, DtFloat *v, DtFloat *w )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiTrimCurve( DtInt nloops, DtInt *ncurves, DtInt *order, DtFloat *knot, DtFloat *amin, DtFloat *amax, DtInt *n, DtFloat *u, DtFloat *v, DtFloat *w )"); }


    // ------------------ Procedural primitives
    virtual DtVoid  DiProcedural( DtPointer data, DtBound bound, DtVoid (*subdivfunc) (DtPointer, DtFloat), DtVoid (*freefunc) (DtPointer) )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiProcedural( DtPointer data, DtBound bound, DtVoid (*subdivfunc) (DtPointer, DtFloat), DtVoid (*freefunc) (DtPointer) )"); }
    virtual DtVoid  DiProcDelayedReadArchive (DtPointer data, DtFloat detail)   { dzApp->log("ReadyKitRenderer: unimplemented method: DiProcDelayedReadArchive (DtPointer data, DtFloat detail)"); }
    virtual DtVoid  DiProcRunProgram (DtPointer data, DtFloat detail)   { dzApp->log("ReadyKitRenderer: unimplemented method: DiProcRunProgram (DtPointer data, DtFloat detail)"); }
    virtual DtVoid  DiProcDynamicLoad (DtPointer data, DtFloat detail)  { dzApp->log("ReadyKitRenderer: unimplemented method: DiProcDynamicLoad (DtPointer data, DtFloat detail)"); }


    // ------------------ Object Instancing
    virtual DtObjectHandle  DiObjectBegin() { dzApp->log("ReadyKitRenderer: unimplemented method: DiObjectBegin()"); }
    virtual DtObjectHandle  DiObjectBeginV( DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiObjectBeginV( DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiObjectEnd()   { dzApp->log("ReadyKitRenderer: unimplemented method: DiObjectEnd()"); }
    virtual DtVoid  DiObjectInstance( DtObjectHandle handle )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiObjectInstance( DtObjectHandle handle )"); }


    // ------------------ Resourcing
    virtual DtVoid  DiResourceV(DtToken handle, DtToken type, DtInt n, const DtToken tokens[], DtPointer params[])  { dzApp->log("ReadyKitRenderer: unimplemented method: DiResourceV(DtToken handle, DtToken type, DtInt n, const DtToken tokens[], DtPointer params[])"); }
    virtual DtVoid  DiResourceBegin()   { dzApp->log("ReadyKitRenderer: unimplemented method: DiResourceBegin()"); }
    virtual DtVoid  DiResourceEnd() { dzApp->log("ReadyKitRenderer: unimplemented method: DiResourceEnd()"); }


    // ------------------ Texture map creation */
    virtual DtVoid  DiMakeBrickMapV( DtInt nptc, const char *const *ptcnames, const char *bkmname, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiMakeBrickMapV( DtInt nptc, const char *const *ptcnames, const char *bkmname, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiMakeBumpV( const char *picturename, const char *texturename, DtToken swrap, DtToken twrap, DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiMakeBumpV( const char *picturename, const char *texturename, DtToken swrap, DtToken twrap, DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiMakeCubeFaceEnvironmentV( const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *tex, DtFloat fov, DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiMakeCubeFaceEnvironmentV( const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *tex, DtFloat fov, DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiMakeLatLongEnvironmentV( const char *pic, const char *tex, DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiMakeLatLongEnvironmentV( const char *pic, const char *tex, DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiMakeShadowV( const char *pic, const char *tex, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiMakeShadowV( const char *pic, const char *tex, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiMakeTextureV( const char *pic, const char *tex, DtToken swrap, DtToken twrap, DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiMakeTextureV( const char *pic, const char *tex, DtToken swrap, DtToken twrap, DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, DtInt n, const DtToken tokens[], DtPointer params[] )"); }


    // ------------------ Reading and writing archive files
    virtual DtVoid  DiArchiveRecord( DtToken type, const char *format, ... ){ dzApp->log("ReadyKitRenderer: unimplemented method: DiArchiveRecord( DtToken type, const char *format, ... )"); }
    virtual DtVoid  DiReadArchiveV( DtString filename, DtArchiveCallback callback, int n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiReadArchiveV( DtString filename, DtArchiveCallback callback, int n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtArchiveHandle DiArchiveBeginV( DtToken archivename, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiArchiveBeginV( DtToken archivename, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiArchiveEnd()  { dzApp->log("ReadyKitRenderer: unimplemented method: DiArchiveEnd()"); }


    // ------------------ Renderer options
    virtual DtVoid  DiCameraV( DtToken camera, DtInt n, const DtToken tokens[], DtPointer params[] )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiCameraV( DtToken camera, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiClipping( DtFloat hither, DtFloat yon )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiClipping( DtFloat hither, DtFloat yon )"); }
    virtual DtVoid  DiClippingPlane( DtFloat x, DtFloat y, DtFloat z, DtFloat nx, DtFloat ny, DtFloat nz )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiClippingPlane( DtFloat x, DtFloat y, DtFloat z, DtFloat nx, DtFloat ny, DtFloat nz )"); }
    virtual DtVoid  DiColorSamples( DtInt N, DtFloat *nRGB, DtFloat *RGBn ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiColorSamples( DtInt N, DtFloat *nRGB, DtFloat *RGBn )"); }
    virtual DtVoid  DiCropWindow( DtFloat xmin, DtFloat xmax, DtFloat ymin, DtFloat ymax )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiCropWindow( DtFloat xmin, DtFloat xmax, DtFloat ymin, DtFloat ymax )"); }
    virtual DtVoid  DiDepthOfField( DtFloat fstop, DtFloat focallength, DtFloat focaldistance ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiDepthOfField( DtFloat fstop, DtFloat focallength, DtFloat focaldistance )"); }
    virtual DtVoid  DiDisplayV( const char *name, DtToken type, DtToken mode, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiDisplayV( const char *name, DtToken type, DtToken mode, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiDisplayChannelV( const char *channel, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiDisplayChannelV( const char *channel, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiExposure( DtFloat gain, DtFloat gamma )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiExposure( DtFloat gain, DtFloat gamma )"); }
    virtual DtVoid  DiFormat( DtInt xres, DtInt yres, DtFloat aspect )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiFormat( DtInt xres, DtInt yres, DtFloat aspect )"); }
    virtual DtVoid  DiFrameAspectRatio( DtFloat aspect )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiFrameAspectRatio( DtFloat aspect )"); }
    virtual DtVoid  DiHiderV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] )   { dzApp->log("ReadyKitRenderer: unimplemented method: DiHiderV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiOptionV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiOptionV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiPixelFilter( DtFilterFunc function, DtFloat xwidth, DtFloat ywidth )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiPixelFilter( DtFilterFunc function, DtFloat xwidth, DtFloat ywidth )"); }
    virtual DtVoid  DiPixelSamples( DtFloat xsamples, DtFloat ysamples )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiPixelSamples( DtFloat xsamples, DtFloat ysamples )"); }
    virtual DtVoid  DiPixelVariance( DtFloat variation )    { dzApp->log("ReadyKitRenderer: unimplemented method: DiPixelVariance( DtFloat variation )"); }
    virtual DtVoid  DiProjectionV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiProjectionV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )"); }
    virtual DtVoid  DiQuantize( DtToken type, DtInt one, DtInt qmin, DtInt qmax, DtFloat ampl ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiQuantize( DtToken type, DtInt one, DtInt qmin, DtInt qmax, DtFloat ampl )"); }
    virtual DtVoid  DiRelativeDetail( DtFloat relativedetail )  { dzApp->log("ReadyKitRenderer: unimplemented method: DiRelativeDetail( DtFloat relativedetail )"); }
    virtual DtVoid  DiShutter( DtFloat smin, DtFloat smax ) { dzApp->log("ReadyKitRenderer: unimplemented method: DiShutter( DtFloat smin, DtFloat smax )"); }


    // ------------------ Shader Language Interrogation
    virtual int DSlo_SetPath( const char *i_path )    { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_SetPath( const char *i_path )"); }
    virtual int DSlo_SetShader( const char *i_name )  { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_SetShader( const char *i_name )"); }
    virtual const char* DSlo_GetName()  { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetName()"); return "ReadyKitRenderer RenderKit"; }
    virtual DSLO_TYPE   DSlo_GetType()  { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetType()"); }
    virtual int DSlo_HasMethod( const char *i_name )  { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_HasMethod( const char *i_name )"); }
    virtual const char* const*	DSlo_GetMethodNames(){ dzApp->log("ReadyKitRenderer: unimplemented DSlo call"); };
    virtual int DSlo_GetNArgs() { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetNArgs()"); }

    virtual DSLO_VISSYMDEF* DSlo_GetArgById( int i_id ) { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetArgById( int i_id )"); }
    virtual DSLO_VISSYMDEF* DSlo_GetArgByName( const char *i_name ) { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetArgByName( const char *i_name )"); }
    virtual DSLO_VISSYMDEF* DSlo_GetArrayArgElement( DSLO_VISSYMDEF *i_array, int i_index ) { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetArrayArgElement( DSLO_VISSYMDEF *i_array, int i_index )"); }

    virtual int DSlo_GetNAnnotations()  { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetNAnnotations()"); }
    virtual const char* DSlo_GetAnnotationKeyById( int i_id )   { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetAnnotationKeyById( int i_id )"); }
    virtual const char* DSlo_GetAnnotationByKey( const char *i_key )    { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_GetAnnotationByKey( const char *i_key )"); }

    virtual void    DSlo_EndShader()    { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_EndShader()"); }
    virtual const char* DSlo_TypetoStr( DSLO_TYPE i_type )  { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_TypetoStr( DSLO_TYPE i_type )"); }
    virtual const char* DSlo_StortoStr( DSLO_STORAGE i_storage )    { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_StortoStr( DSLO_STORAGE i_storage )"); }
    virtual const char* DSlo_DetailtoStr( DSLO_DETAIL i_detail )    { dzApp->log("ReadyKitRenderer: unimplemented method: DSlo_DetailtoStr( DSLO_DETAIL i_detail )"); }


    // ------------------ Rendering
    virtual bool    render( DzRenderHandler *handler, DzCamera *camera, const DzRenderOptions &opt )    { dzApp->log("ReadyKitRenderer: unimplemented method: render( DzRenderHandler *handler, DzCamera *camera, const DzRenderOptions &opt )"); }
    virtual bool    customRender( DzRenderHandler *handler, DzCamera *camera, DzLightList &lights, DzNodeList &nodes, const DzRenderOptions &opt )  { dzApp->log("ReadyKitRenderer: unimplemented method: customRender( DzRenderHandler *handler, DzCamera *camera, DzLightList &lights, DzNodeList &nodes, const DzRenderOptions &opt )"); }

    //////////////////////////////////////////////////////////
    //////////// ********* public slots ********//////////////
    //////////////////////////////////////////////////////////
    public slots:

    //
    // MANIPULATORS
    //

    bool    render( DzRenderHandler *handler, DzCamera *camera, const DzRenderOptions *opt );
    bool    customRender( DzRenderHandler *handler, DzCamera *camera, QObjectList lights, QObjectList nodes, const DzRenderOptions *opt );
    virtual void    prepareImage( const DzTexture *img, const QString &filename ) ;
    virtual QString compileShader( const QString &shaderPath ) ;
    virtual QString compileShader( const QString &shaderPath, QString &output ) ;
    virtual DzShaderDescription*    getShaderInfo( const QString &shaderPath );
    virtual void    killRender();

    virtual bool    bake( DzRenderHandler *handler, DzCamera *camera, DzLightListIterator &lights, DzNodeListIterator &nodes, const DzBakerOptions &opt );
    virtual bool    autoBake( DzRenderHandler *handler, DzCamera *camera, DzLightListIterator &lights, DzNodeListIterator &nodes, const DzBakerOptions &opt );
    virtual void    stopBaking();
    virtual void    saveBakeImage( const DzBakerOptions &opt, bool wait );

    virtual bool    textureConvert( DzRenderHandler *handler, DzCamera *camera, const DzTextureConvertorOptions &opt );
    //
    // ACCESSORS
    //

    virtual QString getShaderCompilerPath();
    virtual QString getTextureUtilityPath();

    virtual QStringList getShaderSearchPaths() const;
    virtual QString processShaderName( const QString &shaderName ) const;
    virtual QString getShaderPath( const QString &shaderName, bool withExtension = true ) const;
    virtual QString getShaderFileName( const QString &shaderName ) const;
    virtual QString getShaderExtension() const;
    virtual bool    isRendering() const;
    virtual QString getName() const;
    virtual DzNode* getCurrentNode() const;

    // ReadyKitRenderer slots
    void    handlePrepareImageComplete(  WorkerPrepareImage *worker, const DzTexture *img, const QString &filename );
    void    handleRenderProcessComplete( int exitCode, QProcess::ExitStatus status );
    void    updateData();


public:

    virtual DzOptionsFrame*	getOptionsFrame() const ;
    virtual DtFilterFunc    getFilterFunction( DzRenderOptions::PixelFilter filterType ) const ;

signals:

    void    aboutToRender( DzRenderer *r ) ;
    void    renderFinished( DzRenderer *r ) ;
    void    renderFinished();
    void    imagePrepared( const DzTexture *img, const QString &filename ) ;
    void    updateData( const DzRenderData &data );
    void    beginningRender();
    void    beginningFrame(int frame);
    void    frameFinished();


protected:

    // ------------------ Basic control flow, scoping, stacks

    virtual DtVoid  DiBegin( DtToken name ) ;
    virtual DtVoid  DiEnd() ;
    virtual DtVoid  DiFrameBegin( DtInt number ) ;
    virtual DtVoid  DiFrameEnd() ;

    //void  renderRecurse( DzNode *node ){ dzApp->log("ReadyKitRenderer: unimplemented"); };
    //static void   errorFunc( DtInt code, DtInt severity, char *msg ){ dzApp->log("ReadyKitRenderer: unimplemented"); };

private:

    //struct    Data;
    //Data  *m_data;



    
};



#endif // ReadyKitRenderer_h
