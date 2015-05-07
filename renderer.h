//
//  renderer.h
//  yaluxpug
//
//  Created by Daniel Bui on 4/23/15.
//
//

#ifndef yaluxplug_renderer_h
#define yaluxplug_renderer_h

#include "plugin.h"
#include "dzrenderer.h"
#include "dzrenderdata.h"
#include "dzapp.h"
#include <QtCore/QStringList>
#include <QtCore/QProcess>

class YaLuxRender : public DzRenderer {
    Q_OBJECT
public:
    YaLuxRender();

    
    // ------------------ Basic control flow, scoping, stacks
    
    virtual DtVoid			DiWorldBegin() { dzApp->log("yaluxplug: unimplemented basic control1"); };
    virtual DtVoid			DiWorldEnd() { dzApp->log("yaluxplug: unimplemented basic control2"); };
    virtual DtContextHandle DiGetContext() { dzApp->log("yaluxplug: unimplemented basic control"); };
    virtual DtVoid			DiContext( DtContextHandle handle ) { dzApp->log("yaluxplug: unimplemented basic control3"); };
    virtual DtVoid			DiAttributeBegin() { dzApp->log("\nyaluxplug: unimplemented basic DiAttributeBegin()"); };
    virtual DtVoid			DiAttributeEnd() { dzApp->log("yaluxplug: unimplemented basic DiAttributeEnd()\n"); };
    virtual DtVoid			DiTransformBegin() { dzApp->log("yaluxplug: unimplemented basic DiTransformBegin()"); };
    virtual DtVoid			DiTransformEnd() { dzApp->log("yaluxplug: unimplemented basic DiTransformEnd()"); };
    virtual DtVoid			DiSolidBegin( DtToken type ) { dzApp->log("yaluxplug: unimplemented basic control8"); };
    virtual DtVoid			DiSolidEnd() { dzApp->log("yaluxplug: unimplemented basic control9"); };
    //DtVoid					DiMotionBegin( DtInt N, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiMotionBeginV( DtInt N, DtFloat times[] ) { dzApp->log("yaluxplug: unimplemented basic control10"); };
    virtual DtVoid			DiMotionEnd() { dzApp->log("yaluxplug: unimplemented basic control11"); };
    virtual DtToken			DiDeclare( const char *name, const char *declaration );
    virtual DtVoid			DiScreenWindow( DtFloat left, DtFloat right, DtFloat bot, DtFloat top ) { dzApp->log("yaluxplug: unimplemented basic control13"); };
    //DtVoid					DiIfBegin( DtToken expression, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiIfBeginV( DtToken expression, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("yaluxplug: unimplemented basic control14"); };
    //DtVoid					DiElseIf( DtToken expression, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiElseIfV( DtToken expression, DtInt n, const DtToken tokens[], DtPointer params[] ) { dzApp->log("yaluxplug: unimplemented basic control15"); };
    virtual DtVoid			DiElse() { dzApp->log("yaluxplug: unimplemented basic control16"); };
    virtual DtVoid			DiIfEnd(){ dzApp->log("yaluxplug: unimplemented basic control17"); };
    virtual DtVoid			DiErrorHandler( DtErrorHandler handler ){ dzApp->log("yaluxplug: unimplemented basic control18"); };
    virtual DtVoid			DiSynchronize( DtToken ){ dzApp->log("yaluxplug: unimplemented basic control19"); };
    
    
    // ------------------ Attributes
    
    //DtVoid					DiAttribute( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiAttributeV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] );
    virtual DtVoid			DiBound( DtBound bound ){ dzApp->log("yaluxplug: unimplemented attribute call2"); };
    virtual DtVoid			DiColor( DtColor Cs );
    virtual DtVoid			DiDetail( DtBound bound ){ dzApp->log("yaluxplug: unimplemented attribute call4"); };
    virtual DtVoid			DiDetailRange( DtFloat minvis, DtFloat lowtran,
                                          DtFloat uptran, DtFloat maxvis ){ dzApp->log("yaluxplug: unimplemented attribute call5"); };
    virtual DtVoid			DiGeometricApproximation( DtToken type, DtFloat value ){ dzApp->log("yaluxplug: unimplemented attribute call6"); };
    virtual DtVoid			DiIlluminate( DtLightHandle light, DtBoolean onoff ){ dzApp->log("yaluxplug: unimplemented attribute call7"); };
    virtual DtVoid			DiMatte( DtBoolean onoff ){ dzApp->log("yaluxplug: unimplemented attribute call8"); };
    virtual DtVoid			DiMultiplyShadingRate( DtFloat ratemultiplier ){ dzApp->log("yaluxplug: unimplemented attribute call9"); };
    virtual DtVoid			DiOpacity( DtColor Cs );
    virtual DtVoid			DiOrientation( DtToken orientation ){ dzApp->log("yaluxplug: unimplemented attribute call11"); };
    virtual DtVoid			DiReverseOrientation(){ dzApp->log("yaluxplug: unimplemented attribute call12"); };
    virtual DtVoid			DiShadingInterpolation( DtToken type ){ dzApp->log("yaluxplug: unimplemented attribute call13"); };
    virtual DtVoid			DiShadingRate( DtFloat size ){ dzApp->log("yaluxplug: unimplemented attribute call14"); };
    virtual DtVoid			DiSides( DtInt nsides ){ dzApp->log("yaluxplug: unimplemented attribute call15"); };
    virtual DtVoid			DiTextureCoordinates( DtFloat s1, DtFloat t1, DtFloat s2, DtFloat t2,
                                                 DtFloat s3, DtFloat t3, DtFloat s4, DtFloat t4 ){ dzApp->log("yaluxplug: unimplemented attribute call16"); };
    
    
    // ------------------ Shaders
    
    //DtLightHandle			DiAreaLightSource( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtLightHandle	DiAreaLightSourceV( DtToken name, DtInt n,
                                               const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders call1"); };
    //DtVoid					DiAtmosphere( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiAtmosphereV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders call2"); };
    //DtVoid					DiDisplacement( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiDisplacementV( DtToken name, DtInt n,
                                            const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders DiDisplacementV()"); };
    //DtVoid					DiExterior( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiExteriorV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders call4"); };
    //DtVoid					DiImager( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiImagerV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders call5"); };
    //DtVoid					DiInterior( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiInteriorV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders call6"); };
    //DtLightHandle			DiLightSource( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtLightHandle	DiLightSourceV( DtToken name, DtInt n,
                                           const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders call7"); };
    //DtVoid					DiShader( DtToken name, DtToken handle, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiShaderV( DtToken name, DtToken handle, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders call8"); };
    //DtVoid					DiSurface( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiSurfaceV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented shaders DiSurfaceV()"); };
    
    
    // ------------------ Transformations
    
    virtual DtVoid			DiConcatTransform( DtMatrix transform ){ dzApp->log("yaluxplug: unimplemented transformation call1"); };
    virtual DtVoid			DiCoordinateSystem( DtToken space ){ dzApp->log("yaluxplug: unimplemented transformation call2"); };
    virtual DtVoid			DiCoordSysTransform( DtToken space ){ dzApp->log("yaluxplug: unimplemented transformation call3"); };
    virtual DtVoid			DiIdentity(){ dzApp->log("yaluxplug: unimplemented transformation call4"); };
    virtual DtVoid			DiPerspective( DtFloat fov ){ dzApp->log("yaluxplug: unimplemented transformation call5"); };
    virtual DtVoid			DiRotate( DtFloat angle, DtFloat dx, DtFloat dy, DtFloat dz ){ dzApp->log("yaluxplug: unimplemented transformation call6"); };
    virtual DtVoid			DiScale( DtFloat dx, DtFloat dy, DtFloat dz ){ dzApp->log("yaluxplug: unimplemented transformation call7"); };
    virtual DtVoid			DiScopedCoordinateSystem( DtToken space ){ dzApp->log("yaluxplug: unimplemented transformation call8"); };
    virtual DtVoid			DiSkew( DtFloat angle, DtFloat dx1, DtFloat dy1, DtFloat dz1,
                                   DtFloat dx2, DtFloat dy2, DtFloat dz2 ){ dzApp->log("yaluxplug: unimplemented transformation call9"); };
    virtual DtVoid			DiTransform( DtMatrix transform );
    virtual DtPoint*		DiTransformPoints( DtToken fromspace, DtToken tospace,
                                              DtInt npoints, DtPoint *points ){ dzApp->log("yaluxplug: unimplemented transformation call11"); };
    virtual DtVoid			DiTranslate( DtFloat dx, DtFloat dy, DtFloat dz ){ dzApp->log("yaluxplug: unimplemented transformation call12"); };
    
    
    // ------------------ Geometric Primitives
    
    virtual DtVoid			DiBasis( DtBasis ubasis, DtInt ustep, DtBasis vbasis, DtInt vstep ){ dzApp->log("yaluxplug: unimplemented prim geometry call1"); };
    //DtVoid					DiBlobby( DtInt nleaf, DtInt ncode, DtInt code[],
    //                                 DtInt nflt, DtFloat flt[], DtInt nstr, DtString str[], ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiBlobbyV( DtInt nleaf, DtInt ncode, DtInt code[],
                                      DtInt nflt, DtFloat flt[], DtInt nstr, DtString str[],
                                      DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call2"); };
    //DtVoid					DiCone( DtFloat height, DtFloat radius, DtFloat thetamax, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiConeV( DtFloat height, DtFloat radius, DtFloat thetamax,
                                    DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call3"); };
    //DtVoid					DiCurves( DtToken degree, DtInt ncurves,
    //                                 DtInt nverts[], DtToken wrap, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiCurvesV( DtToken degree, DtInt ncurves, DtInt nverts[], DtToken wrap,
                                      DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call4"); };
    //DtVoid					DiCylinder( DtFloat radius, DtFloat zmin, DtFloat zmax,
    //                                   DtFloat thetamax, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiCylinderV( DtFloat radius, DtFloat zmin, DtFloat zmax, DtFloat thetamax,
                                        DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call5"); };
    //DtVoid					DiDisk( DtFloat height, DtFloat radius, DtFloat thetamax, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiDiskV( DtFloat height, DtFloat radius, DtFloat thetamax,
                                    DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call6"); };
    //DtVoid					DiGeneralPolygon( DtInt nloops, DtInt *nverts, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiGeneralPolygonV (DtInt nloops, DtInt *nverts,
                                               DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call7"); };
    //DtVoid					DiGeometry( DtToken type, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiGeometryV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call8"); };
    //DtVoid					DiHyperboloid( DtPoint point1, DtPoint point2, DtFloat thetamax, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiHyperboloidV( DtPoint point1, DtPoint point2, DtFloat thetamax,
                                           DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call9"); };
    //DtVoid					DiNuCurves( DtInt ncurves, DtInt nvertices[],
    //                                   DtInt order[], DtFloat knot[], DtFloat min[], DtFloat max[], ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiNuCurvesV( DtInt ncurves, DtInt nvertices[], 
                                        DtInt order[], DtFloat knot[], DtFloat min[], DtFloat max[],
                                        DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call10"); };
    //DtVoid					DiNuPatch( DtInt nu, DtInt uorder, DtFloat *uknot,
    //                                  DtFloat umin, DtFloat umax, DtInt nv, DtInt vorder,
    //                                  DtFloat *vknot, DtFloat vmin, DtFloat vmax, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiNuPatchV( DtInt nu, DtInt uorder, DtFloat *uknot,
                                       DtFloat umin, DtFloat umax, DtInt nv, DtInt vorder,
                                       DtFloat *vknot, DtFloat vmin, DtFloat vmax,
                                       DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call11"); };
    //DtVoid					DiParaboloid( DtFloat rmax, DtFloat zmin,
    //                                     DtFloat zmax, DtFloat thetamax, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiParaboloidV( DtFloat rmax, DtFloat zmin, DtFloat zmax, DtFloat thetamax,
                                          DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call12"); };
    //DtVoid					DiPatch( DtToken type, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiPatchV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call13"); };
    //DtVoid					DiPatchMesh( DtToken type, DtInt nu, DtToken uwrap,
    //                                    DtInt nv, DtToken vwrap, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiPatchMeshV( DtToken type, DtInt nu, DtToken uwrap, DtInt nv,
                                         DtToken vwrap, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call14"); };
    //DtVoid					DiPoints( DtInt npts, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiPointsV( DtInt npts, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call15"); };
    //DtVoid					DiPointsGeneralPolygons( DtInt npolys, DtInt *nloops,
    //                                                DtInt *nverts, DtInt *verts, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiPointsGeneralPolygonsV( DtInt npolys, DtInt *nloops,
                                                     DtInt *nverts, DtInt *verts,
                                                     DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call16"); };
    //DtVoid					DiPointsPolygons( DtInt npolys, DtInt *nverts, DtInt *verts, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiPointsPolygonsV( DtInt npolys, DtInt *nverts, DtInt *verts,
                                              DtInt n, const DtToken tokens[], DtPointer params[] );
    //DtVoid					DiPolygon( DtInt nvertices, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiPolygonV( DtInt nvertices, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call18"); };
    //DtVoid					DiSphere( DtFloat radius, DtFloat zmin,
    //                                 DtFloat zmax, DtFloat thetamax, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiSphereV( DtFloat radius, DtFloat zmin, DtFloat zmax, DtFloat thetamax,
                                      DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call19"); };
    //DtVoid					DiSubdivisionMesh( DtToken scheme, DtInt nfaces,
    //                                          DtInt nvertices[], DtInt vertices[],
    //                                          DtInt ntags, const DtToken tags[], DtInt nargs[],
    //                                          DtInt intargs[], DtFloat floatargs[], ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiSubdivisionMeshV( DtToken scheme, DtInt nfaces,
                                               DtInt nvertices[], DtInt vertices[],
                                               DtInt ntags, const DtToken tags[], DtInt nargs[],
                                               DtInt intargs[], DtFloat floatargs[],
                                               DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call20"); };
    //DtVoid					DiHierarchicalSubdivisionMesh(	DtToken scheme,
    //                                                      DtInt nfaces, DtInt nvertices[], DtInt vertices[],
    //                                                      DtInt ntags, const DtToken tags[], DtInt nargs[],
    //                                                      DtInt intargs[], DtFloat floatargs[], const DtToken stringargs[], ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiHierarchicalSubdivisionMeshV(	DtToken scheme,
                                                           DtInt nfaces, DtInt nvertices[], DtInt vertices[],
                                                           DtInt ntags, const DtToken tags[], DtInt nargs[],
                                                           DtInt intargs[], DtFloat floatargs[], const DtToken stringargs[],
                                                           DtInt n, const DtToken tokens[], DtPointer params[]);
    //DtVoid					DiTorus( DtFloat majorrad, DtFloat minorrad, DtFloat phimin,
    //                                DtFloat phimax, DtFloat thetamax, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiTorusV( DtFloat majorrad, DtFloat minorrad, DtFloat phimin,
                                     DtFloat phimax, DtFloat thetamax,
                                     DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented prim geometry call22"); };
    virtual DtVoid			DiTrimCurve( DtInt nloops, DtInt *ncurves, DtInt *order,
                                        DtFloat *knot, DtFloat *amin, DtFloat *amax,
                                        DtInt *n, DtFloat *u, DtFloat *v, DtFloat *w ){ dzApp->log("yaluxplug: unimplemented prim geometry call23"); };
    
    
    // ------------------ Procedural primitives
    
    virtual DtVoid			DiProcedural( DtPointer data, DtBound bound,
                                         DtVoid (*subdivfunc) (DtPointer, DtFloat),
                                         DtVoid (*freefunc) (DtPointer) ){ dzApp->log("yaluxplug: unimplemented procedure call"); };
    virtual DtVoid			DiProcDelayedReadArchive (DtPointer data, DtFloat detail){ dzApp->log("yaluxplug: unimplemented procedure call"); };
    virtual DtVoid			DiProcRunProgram (DtPointer data, DtFloat detail){ dzApp->log("yaluxplug: unimplemented procedure call"); };
    virtual DtVoid			DiProcDynamicLoad (DtPointer data, DtFloat detail){ dzApp->log("yaluxplug: unimplemented procedure call"); };
    
    
    // ------------------ Object Instancing
    
    virtual DtObjectHandle	DiObjectBegin(){ dzApp->log("yaluxplug: unimplemented object call"); };
    virtual DtObjectHandle	DiObjectBeginV( DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented object call"); };
    virtual DtVoid			DiObjectEnd(){ dzApp->log("yaluxplug: unimplemented object call"); };
    virtual DtVoid			DiObjectInstance( DtObjectHandle handle ){ dzApp->log("yaluxplug: unimplemented object call"); };
    
    
    // ------------------ Resourcing
    
    //DtVoid					DiResource(DtToken handle, DtToken type, ...){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiResourceV(DtToken handle, DtToken type,
                                        DtInt n, const DtToken tokens[], DtPointer params[]);
    virtual DtVoid			DiResourceBegin(){ dzApp->log("yaluxplug: unimplemented resource call2"); };
    virtual DtVoid			DiResourceEnd(){ dzApp->log("yaluxplug: unimplemented resource call3"); };
    
    
    // ------------------ Texture map creation */
    
    //DtVoid					DiMakeBrickMap( DtInt nptc, const char *const *ptcnames, const char *bkmname, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiMakeBrickMapV( DtInt nptc, const char *const *ptcnames, const char *bkmname,
                                            DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented texture call1"); };
    //DtVoid					DiMakeBump( const char *picturename, const char *texturename,
    //                                   DtToken swrap, DtToken twrap,
    //                                   DtFilterFunc filterfunc,
    //                                   DtFloat swidth, DtFloat twidth, ... ){ dzApp->log("yaluxplug: unimplemented"); };	
    virtual DtVoid			DiMakeBumpV( const char *picturename, const char *texturename,
                                        DtToken swrap, DtToken twrap,
                                        DtFilterFunc filterfunc,
                                        DtFloat swidth, DtFloat twidth,
                                        DtInt n, const DtToken tokens[],
                                        DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented texture call texture call2"); };
    //DtVoid					DiMakeCubeFaceEnvironment( const char *px, const char *nx, const char *py,
    //                                                  const char *ny, const char *pz, const char *nz,
    //                                                  const char *tex, DtFloat fov, DtFilterFunc filterfunc,
    //                                                  DtFloat swidth, DtFloat twidth, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiMakeCubeFaceEnvironmentV( const char *px, const char *nx, const char *py,
                                                       const char *ny, const char *pz, const char *nz,
                                                       const char *tex, DtFloat fov, DtFilterFunc filterfunc,
                                                       DtFloat swidth, DtFloat twidth,
                                                       DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented texture call3"); };
    //DtVoid					DiMakeLatLongEnvironment( const char *pic, const char *tex, DtFilterFunc filterfunc,
    //                                                 DtFloat swidth, DtFloat twidth, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiMakeLatLongEnvironmentV( const char *pic, const char *tex, DtFilterFunc filterfunc,
                                                      DtFloat swidth, DtFloat twidth,
                                                      DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented texture call4"); };
    //DtVoid					DiMakeShadow( const char *pic, const char *tex, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiMakeShadowV( const char *pic, const char *tex,
                                          DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented texture call5"); };
    //DtVoid					DiMakeTexture( const char *pic, const char *tex, DtToken swrap, DtToken twrap,
    //                                      DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiMakeTextureV( const char *pic, const char *tex, DtToken swrap, DtToken twrap,
                                           DtFilterFunc filterfunc, DtFloat swidth, DtFloat twidth,
                                           DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented texture call6"); };
    
    
    // ------------------ Reading and writing archive files
    
    virtual DtVoid			DiArchiveRecord( DtToken type, const char *format, ... ){ dzApp->log("yaluxplug: unimplemented archive call1"); };
    //DtVoid					DiReadArchive( DtString filename, DtArchiveCallback callback, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiReadArchiveV( DtString filename, DtArchiveCallback callback,
                                           int n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented archive call2"); };
    //DtArchiveHandle			DiArchiveBegin( DtToken archivename, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtArchiveHandle	DiArchiveBeginV( DtToken archivename, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented archive call3"); };
    virtual DtVoid			DiArchiveEnd(){ dzApp->log("yaluxplug: unimplemented archive call4"); };
    
    
    // ------------------ Renderer options
    
    //DtVoid					DiCamera( DtToken camera, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiCameraV( DtToken camera, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiClipping( DtFloat hither, DtFloat yon ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiClippingPlane( DtFloat x, DtFloat y, DtFloat z, DtFloat nx, DtFloat ny, DtFloat nz ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiColorSamples( DtInt N, DtFloat *nRGB, DtFloat *RGBn ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiCropWindow( DtFloat xmin, DtFloat xmax, DtFloat ymin, DtFloat ymax ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiDepthOfField( DtFloat fstop, DtFloat focallength, DtFloat focaldistance ){ dzApp->log("yaluxplug: unimplemented render call"); };
    //DtVoid					DiDisplay( const char *name, DtToken type, DtToken mode, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiDisplayV( const char *name, DtToken type, DtToken mode,
                                       DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented render call"); };
    //DtVoid					DiDisplayChannel( const char *channel, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiDisplayChannelV( const char *channel, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiExposure( DtFloat gain, DtFloat gamma ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiFormat( DtInt xres, DtInt yres, DtFloat aspect ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiFrameAspectRatio( DtFloat aspect ){ dzApp->log("yaluxplug: unimplemented render call"); };
    //DtVoid					DiHider( DtToken type, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiHiderV( DtToken type, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented render call"); };
    //DtVoid					DiOption( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiOptionV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiPixelFilter( DtFilterFunc function, DtFloat xwidth, DtFloat ywidth ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiPixelSamples( DtFloat xsamples, DtFloat ysamples ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiPixelVariance( DtFloat variation ){ dzApp->log("yaluxplug: unimplemented render call"); };
    //DtVoid					DiProjection( DtToken name, ... ){ dzApp->log("yaluxplug: unimplemented"); };
    virtual DtVoid			DiProjectionV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiQuantize( DtToken type, DtInt one, DtInt qmin, DtInt qmax, DtFloat ampl ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiRelativeDetail( DtFloat relativedetail ){ dzApp->log("yaluxplug: unimplemented render call"); };
    virtual DtVoid			DiShutter( DtFloat smin, DtFloat smax ){ dzApp->log("yaluxplug: unimplemented render call"); };
    
    
    // ------------------ Shader Language Interrogation
    
    virtual int				DSlo_SetPath(const char *i_path){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual int				DSlo_SetShader(const char *i_name){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual const char*		DSlo_GetName(){ dzApp->log("yaluxplug: unimplemented DSlo call"); return "yaluxplug"; };
    virtual DSLO_TYPE		DSlo_GetType(){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual int				DSlo_HasMethod(const char *i_name){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual const char* const*	DSlo_GetMethodNames(){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual int				DSlo_GetNArgs(){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    
    virtual DSLO_VISSYMDEF*	DSlo_GetArgById(int i_id){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual DSLO_VISSYMDEF*	DSlo_GetArgByName(const char *i_name){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual DSLO_VISSYMDEF*	DSlo_GetArrayArgElement(DSLO_VISSYMDEF *i_array, int i_index){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    
    virtual int				DSlo_GetNAnnotations(){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual const char*		DSlo_GetAnnotationKeyById(int i_id){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual const char*		DSlo_GetAnnotationByKey(const char *i_key){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    
    virtual void			DSlo_EndShader(){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual const char*		DSlo_TypetoStr(DSLO_TYPE i_type){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual const char*		DSlo_StortoStr(DSLO_STORAGE i_storage){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    virtual const char*		DSlo_DetailtoStr(DSLO_DETAIL i_detail){ dzApp->log("yaluxplug: unimplemented DSlo call"); };
    
    
    // ------------------ Rendering
    virtual bool			render( DzRenderHandler *handler, DzCamera *camera, const DzRenderOptions &opt );
    virtual bool			customRender( DzRenderHandler *handler, DzCamera *camera, DzLightList &lights, 
                                         DzNodeList &nodes, const DzRenderOptions &opt );

//////////////////////////////////////////////////////////
//////////// ********* public slots ********//////////////
//////////////////////////////////////////////////////////    
    public slots:
    
    //
    // MANIPULATORS
    //
    
    bool						render( DzRenderHandler *handler, DzCamera *camera, const DzRenderOptions *opt );
    bool						customRender( DzRenderHandler *handler, DzCamera *camera, QObjectList lights, 
                                             QObjectList nodes, const DzRenderOptions *opt );
    virtual void					prepareImage( const DzTexture *img, const QString &filename ) ;
    virtual QString					compileShader( const QString &shaderPath ) ;
    virtual QString					compileShader( const QString &shaderPath, QString &output ) ;
    virtual DzShaderDescription*	getShaderInfo( const QString &shaderPath );
    virtual void					killRender();
    
    virtual bool				bake( DzRenderHandler *handler, DzCamera *camera, DzLightListIterator &lights,
                                     DzNodeListIterator &nodes, const DzBakerOptions &opt );
    virtual bool				autoBake( DzRenderHandler *handler, DzCamera *camera, DzLightListIterator &lights, 
                                         DzNodeListIterator &nodes, const DzBakerOptions &opt );
    virtual void				stopBaking();
    virtual void				saveBakeImage( const DzBakerOptions &opt, bool wait );
    
    virtual bool				textureConvert( DzRenderHandler *handler, DzCamera *camera, 
                                               const DzTextureConvertorOptions &opt );
    //
    // ACCESSORS
    //
    
    virtual QString			getShaderCompilerPath();
    virtual QString			getTextureUtilityPath();
    
    virtual QStringList		getShaderSearchPaths() const;
    virtual QString			processShaderName( const QString &shaderName ) const;
    virtual QString			getShaderPath( const QString &shaderName, bool withExtension = true ) const;
    virtual QString			getShaderFileName( const QString &shaderName ) const;
    virtual QString			getShaderExtension() const;
    virtual bool			isRendering() const;
    virtual QString			getName() const;
    virtual DzNode*			getCurrentNode() const;

    // yaluxplug slots
    void                    handlePrepareImageComplete(  WorkerPrepareImage *worker, const DzTexture *img, const QString &filename );
    void                    handleRenderProcessComplete( int exitCode, QProcess::ExitStatus status );
    void                    updateData();

    
public:
    
    virtual DzOptionsFrame*	getOptionsFrame() const ;
    virtual DtFilterFunc	getFilterFunction( DzRenderOptions::PixelFilter filterType ) const ;
    
signals:
    
    void					aboutToRender( DzRenderer *r ) ;
    void					renderFinished( DzRenderer *r ) ;
    void                    renderFinished();
    void					imagePrepared( const DzTexture *img, const QString &filename ) ;
    void                    updateData( const DzRenderData &data );
    void                    beginningRender();
    void                    beginningFrame(int frame);
    void                    frameFinished();
    
    
protected:
    
    // ------------------ Basic control flow, scoping, stacks
    
    virtual DtVoid			DiBegin( DtToken name ) ;
    virtual DtVoid			DiEnd() ;
    virtual DtVoid			DiFrameBegin( DtInt number ) ;
    virtual DtVoid			DiFrameEnd() ;
    
    //void					renderRecurse( DzNode *node ){ dzApp->log("yaluxplug: unimplemented"); };
    //static void				errorFunc( DtInt code, DtInt severity, char *msg ){ dzApp->log("yaluxplug: unimplemented"); };
    
private:
    
    //struct	Data;
    //Data	*m_data;
    
};





#endif // yaluxplug_renderer_h
