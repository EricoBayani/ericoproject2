#include "3d.h"
#include "poly.h"

int16_t g_sintab[] = {
    0, 402, 803, 1205, 1605, 2005, 2404, 2801, 3196, 3589, 3980, 4369, 4756, 5139, 5519, 5896, 6269, 6639, 7005, 7366, 7723, 8075, 8423, 8765, 9102, 9434, 9759, 10079, 10393, 10701, 11002, 11297, 11585, 11866, 12139, 12406, 12665, 12916, 13159, 13395, 13622, 13842, 14053, 14255, 14449, 14634, 14810, 14978, 15136, 15286, 15426, 15557, 15678, 15790, 15892, 15985, 16069, 16142, 16206, 16260, 16305, 16339, 16364, 16379, 16384, 16379, 16364, 16339, 16305, 16260, 16206, 16142, 16069, 15985, 15892, 15790, 15678, 15557, 15426, 15286, 15136, 14978, 14810, 14634, 14449, 14255, 14053, 13842, 13622, 13395, 13159, 12916, 12665, 12406, 12139, 11866, 11585, 11297, 11002, 10701, 10393, 10079, 9759, 9434, 9102, 8765, 8423, 8075, 7723, 7366, 7005, 6639, 6269, 5896, 5519, 5139, 4756, 4369, 3980, 3589, 3196, 2801, 2404, 2005, 1605, 1205, 803, 402, 0, -402, -803, -1205, -1605, -2005, -2404, -2801, -3196, -3589, -3980, -4369, -4756, -5139, -5519, -5896, -6269, -6639, -7005, -7366, -7723, -8075, -8423, -8765, -9102, -9434, -9759, -10079, -10393, -10701, -11002, -11297, -11585, -11866, -12139, -12406, -12665, -12916, -13159, -13395, -13622, -13842, -14053, -14255, -14449, -14634, -14810, -14978, -15136, -15286, -15426, -15557, -15678, -15790, -15892, -15985, -16069, -16142, -16206, -16260, -16305, -16339, -16364, -16379, -16384, -16379, -16364, -16339, -16305, -16260, -16206, -16142, -16069, -15985, -15892, -15790, -15678, -15557, -15426, -15286, -15136, -14978, -14810, -14634, -14449, -14255, -14053, -13842, -13622, -13395, -13159, -12916, -12665, -12406, -12139, -11866, -11585, -11297, -11002, -10701, -10393, -10079, -9759, -9434, -9102, -8765, -8423, -8075, -7723, -7366, -7005, -6639, -6269, -5896, -5519, -5139, -4756, -4369, -3980, -3589, -3196, -2801, -2404, -2005, -1605, -1205, -803, -402 };

#define DO_MUL( r, c ) out->rows[r].v[c] =                                              \
                           ( (int32_t) a->rows[r].v[0] * (int32_t) b->rows[0].v[c] +    \
                             (int32_t) a->rows[r].v[1] * (int32_t) b->rows[1].v[c] +    \
                             (int32_t) a->rows[r].v[2] * (int32_t) b->rows[2].v[c] +    \
                             (int32_t) a->rows[r].v[3] * (int32_t) b->rows[3].v[c] ) >> \
                           14;

void mtx4_mtx4_mul( mtx4 *a, mtx4 *b, mtx4 *out )
{
    DO_MUL( 0, 0 );
    DO_MUL( 0, 1 );
    DO_MUL( 0, 2 );
    DO_MUL( 0, 3 );
    DO_MUL( 1, 0 );
    DO_MUL( 1, 1 );
    DO_MUL( 1, 2 );
    DO_MUL( 1, 3 );
    DO_MUL( 2, 0 );
    DO_MUL( 2, 1 );
    DO_MUL( 2, 2 );
    DO_MUL( 2, 3 );
    DO_MUL( 3, 0 );
    DO_MUL( 3, 1 );
    DO_MUL( 3, 2 );
    DO_MUL( 3, 3 );
}

void ivec3_mtx4_transform( ivec3 *in, mtx4 *mtx, ivec3 *out )
{
    out->x = ( ( ( in->x * mtx->rows[0].v[0] ) + ( in->y * mtx->rows[1].v[0] ) + ( in->z * mtx->rows[2].v[0] ) ) >> 14 ) + mtx->rows[3].v[0];
    out->y = ( ( ( in->x * mtx->rows[0].v[1] ) + ( in->y * mtx->rows[1].v[1] ) + ( in->z * mtx->rows[2].v[1] ) ) >> 14 ) + mtx->rows[3].v[1];
    out->z = ( ( ( in->x * mtx->rows[0].v[2] ) + ( in->y * mtx->rows[1].v[2] ) + ( in->z * mtx->rows[2].v[2] ) ) >> 14 ) + mtx->rows[3].v[2];
}

void mtx4_ident( mtx4 *out )
{
    for ( int i = 0; i < 4; i++ )
    {
        for ( int j = 0; j < 4; j++ )
        {
            out->rows[i].v[j] = ( i == j ) ? 16384 : 0;
        }
    }
}

// p -> pR -> (pR+T)
// (pR+T).I -> pRI + TI = p + TI ... -TI is what we want?

void mtx4_invert( mtx4 *in, mtx4 *out )
{
    for ( int i = 0; i < 4; i++ )
    {
        out->rows[i].v[3] = 0;
        out->rows[3].v[i] = 0;
    }

    // transpose the rotation component
    for ( int i = 0; i < 3; i++ )
    {
        for ( int j = 0; j < 3; j++ )
        {
            out->rows[i].v[j] = in->rows[j].v[i];
        }
    }

    // generate the inverse translation
    ivec3 t, t2;
    ivec3_negate( (ivec3 *) ( &in->rows[3] ), &t );
    ivec3_mtx4_transform( &t, out, &t2 );
    *( (ivec3 *) ( &out->rows[3] ) ) = t2;
}

void mtx4_from_angles( int16_t x, int16_t y, int16_t z, mtx4 *out )
{
    mtx4 ta, tb, tc;

    mtx4_ident( &ta );
    ta.rows[0].v[0] = ta.rows[1].v[1] = icos( z );
    ta.rows[1].v[0] = isin( z );
    ta.rows[0].v[1] = -ta.rows[1].v[0];

    mtx4_ident( &tb );
    tb.rows[2].v[2] = tb.rows[0].v[0] = icos( y );
    tb.rows[0].v[2] = isin( y );
    tb.rows[2].v[0] = -tb.rows[0].v[2];

    mtx4_mtx4_mul( &ta, &tb, &tc );

    mtx4_ident( &ta );
    ta.rows[1].v[1] = ta.rows[2].v[2] = icos( x );
    ta.rows[2].v[1] = isin( x );
    ta.rows[1].v[2] = -ta.rows[2].v[1];

    mtx4_mtx4_mul( &tc, &ta, out );
}

void mtx4_set_translate( mtx4 *m, ivec3 *t )
{
    *( (ivec3 *) ( &m->rows[3] ) ) = *t;
}

shapeWork *allocShapeWork( Heap *h, shapeDef *sd )
{
    // printf( "Starting shape alloc\n" );
    shapeWork *s = (shapeWork *) heapAlloc( h, sizeof( shapeWork ) );
    s->def = sd;
    s->transformedVerts = (ivec3 *) heapAlloc( h, sizeof( ivec3 ) * sd->vertCt );
    s->projectedVerts = (ivec3 *) heapAlloc( h, sizeof( ivec3 ) * sd->vertCt );
    s->normalsDotLight = (int32_t *) heapAlloc( h, sizeof( int32_t ) * sd->normalCt );
    s->normalsDotCamera = (int32_t *) heapAlloc( h, sizeof( int32_t ) * sd->normalCt );
    s->visPolyCt = 0;
    for ( int32_t i = 0; i < DEPTH_BAND_CT; i++ )
    {
        s->depthBands[i].maxPolys = sd->polyCt;
        s->depthBands[i].polyPtrs = (polyWork **) heapAlloc( h, sizeof( polyWork * ) * sd->polyCt );
    }
    s->visPolys = (polyWork *) heapAlloc( h, sizeof( polyWork ) * sd->polyCt );
    s->sortedPolys = (polyWork **) heapAlloc( h, sizeof( polyWork * ) * sd->polyCt );
    s->envMapTexCoords = (envMapTexCoord *) heapAlloc( h, sizeof( envMapTexCoord ) * sd->vertexNormalCt );
    return ( s );
}

void transformAndProjectPoints( shapeWork *sw )
{
    int ct = sw->def->vertCt;
    ivec3 *ss = sw->def->verts;
    ivec3 *tt = sw->transformedVerts;
    ivec3 *pp = sw->projectedVerts;
    while ( ct-- )
    {
        ivec3_mtx4_transform( ss++, &sw->shapeToCamera, tt );
        project_vert( tt++, pp++ );
    }
}

void processNormals( shapeWork *sw )
{
    int32_t ct = sw->def->normalCt;
    ivec3 *nn = sw->def->normals;
    int32_t *cc = sw->normalsDotCamera;

    if ( !( sw->flags & SW_FLAG_FORCE_POLY_COL ) )
    {
        int32_t *ll = sw->normalsDotLight;
        while ( ct-- )
        {
            *( cc++ ) = ivec3_dot_unscaled( nn, &sw->cameraInShapeSpace );
            *( ll++ ) = ivec3_dot_scaled( nn, &sw->lightInShapeSpace );

            nn++;
        }
    }
    else
    {
        while ( ct-- )
        {
            *( cc++ ) = ivec3_dot_unscaled( nn, &sw->cameraInShapeSpace );
            nn++;
        }
    }
}

#if 0
// determine visibility
// do some crude lighting
// determine poly span count required
void processPolyVis( int id, void* userdata )
{
    shapeWork*      sw = (shapeWork*)userdata;
    polyDef*        pd = &sw->def->polys[ id ];
    planeDef*       pld = &sw->def->planes[ pd->pidx ];
    polyWork*       pw = &sw->visPolys[ id ];
    pw->def = pd;

    // is this poly visible?
    int32_t         above = sw->normalsDotCamera[ pld->nidx ] - pld->dist;
    if( above <= 0 )
    {
        pw->depth = DEPTH_INVIS;
        return;
    }

    // choose depth bucket
    int32_t         depth = 0;
    for( int i = 0 ; i < 3 ; i++ )    
    {
        ivec3*      tv = &sw->transformedVerts[ pd->vidx[ i ] ];
        depth += tv->z;
    }
    depth /= 3;

    depth = ( depth - sw->zmin ) * DEPTH_BAND_CT / sw->zrange;
    if( depth < 0 )                 depth = 0;
    if( depth >= DEPTH_BAND_CT )    depth = DEPTH_BAND_CT-1;

    pw->depth = depth;

    if( sw->flags & SW_FLAG_FORCE_POLY_COL )
    {
        pw->col = sw->forcePolyCol;
    }
    else
    {
        int32_t bright = sw->normalsDotLight[ pld->nidx ] >> 6;
        if( bright < 0 )    bright = 0;
        if( bright > 255)   bright = 255;
        pw->col = sw->lightPalette[ bright ];
    }
}
#endif

void processVis( shapeWork *sw )
{
    int32_t ct = sw->def->polyCt;
    polyDef *pd = sw->def->polys;
    polyWork *pw = sw->visPolys;

    while ( ct-- )
    {
        planeDef *pld = &sw->def->planes[pd->pidx];
        pw->def = pd;

        // is this poly visible?
        int32_t above = sw->normalsDotCamera[pld->nidx] - pld->dist;
        if ( above <= 0 )
        {
            pw->depth = DEPTH_INVIS;
        }
        else
        {
            // choose depth bucket
            int32_t depth = 0;
            for ( int i = 0; i < 3; i++ )
            {
                ivec3 *tv = &sw->transformedVerts[pd->vidx[i]];
                depth += tv->z;
            }
            depth /= 3;

            depth = ( depth - sw->zmin ) * DEPTH_BAND_CT / sw->zrange;
            if ( depth < 0 )
                depth = 0;
            if ( depth >= DEPTH_BAND_CT )
                depth = DEPTH_BAND_CT - 1;

            pw->depth = depth;

            if ( sw->flags & SW_FLAG_FORCE_POLY_COL )
            {
                pw->col = sw->forcePolyCol;
            }
            else
            {
                int32_t bright = sw->normalsDotLight[pld->nidx] >> 6;
                if ( bright < 0 )
                    bright = 0;
                if ( bright > 255 )
                    bright = 255;
                pw->col = sw->lightPalette[bright];
            }
        }

        pd++;
        pw++;
    }
}

void plot_line( int x0, int y0, int x1, int y1, uint8_t col )
{
    int dx = abs( x1 - x0 ), sx = x0 < x1 ? 1 : -1;
    int dy = -abs( y1 - y0 ), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    uint8_t *ptr = g_sc.writepage;
    uint8_t *rwp = g_renderWindow;

    ptr += ( y0 * SCR_X ) + x0;
    rwp += ( y0 * SCR_X ) + x0;
    while ( true )
    {
        if ( x0 >= 0 && x0 < SCR_X && y0 >= 0 && y0 < SCR_Y )
        {
            if ( *rwp == 1 )
                *ptr = col;
            else
                *ptr = 0;
        }
        if ( x0 == x1 && y0 == y1 )
        {
            break;
        }
        e2 = 2 * err;
        if ( e2 >= dy )
        {
            err += dy;
            x0 += sx;
            ptr += sx;
            rwp += sx;
        } /* e_xy+e_x > 0 */
        if ( e2 <= dx )
        {
            err += dx;
            y0 += sy;
            ptr += sy * SCR_X;
            rwp += sy * SCR_X;
        } /* e_xy+e_y < 0 */
    }
}

void plot_line2( int x0, int y0, int x1, int y1, uint8_t col )
{
    int32_t dx = x1 - x0;
    int32_t dy = y1 - y0;
    int32_t gr, cp, ct;
    uint8_t *pp, *rwp;

    if ( x0 < 0 || y0 < 0 || x0 >= SCR_X || y0 >= SCR_X ||
         x1 < 0 || y1 < 0 || x1 >= SCR_X || y1 >= SCR_X )
    {
        // eh... just use the slower one...
        plot_line( x0, y0, x1, y1, col );
        return;
    }

    if ( dx * dx > dy * dy )
    {
        // mostly horizontal
        if ( dx < 0 )
        {
            x0 = x1;
            y0 = y1;
            dx = -dx;
            dy = -dy;
        }
        pp = &g_sc.writepage[y0 * SCR_X + x0];
        rwp = &g_renderWindow[y0 * SCR_X + x0];
        ct = dx + 1;
        if ( dy > 0 )
        {
            gr = ( dy << 16 ) / dx;
            cp = 0x8000;
            while ( ct-- )
            {
                if ( *rwp == 1 )
                    *( pp++ ) = col;
                else
                    pp++;

                rwp++;
                cp += gr;
                if ( cp >= 0x10000 )
                {
                    rwp += SCR_X;
                    pp += SCR_X;
                    cp -= 0x10000;
                }
            }
        }
        else
        {
            gr = ( -dy << 16 ) / dx;
            cp = 0x8000;
            while ( ct-- )
            {
                if ( *rwp == 1 )
                    *( pp++ ) = col;
                else
                    pp++;
                rwp++;
                cp += gr;
                if ( cp >= 0x10000 )
                {
                    pp -= SCR_X;
                    rwp -= SCR_X;
                    cp -= 0x10000;
                }
            }
        }
    }
    else
    {
        // mostly vertical
        if ( dy < 0 )
        {
            x0 = x1;
            y0 = y1;
            dx = -dx;
            dy = -dy;
        }
        gr = ( dx << 16 ) / dy;
        cp = ( x0 << 16 ) + 0x8000;
        ct = dy + 1;
        pp = &g_sc.writepage[y0 * SCR_X];
        rwp = &g_renderWindow[y0 * SCR_X];
        while ( ct-- )
        {
            if ( *( rwp + ( cp >> 16 ) ) == 1 )
                *( pp + ( cp >> 16 ) ) = col;
            cp += gr;
            pp += SCR_X;
            rwp += SCR_X;
        }
    }
}

void generateTexCoords( shapeWork *sw )
{
    sw->shapeToWorldNoXlate = sw->shapeToWorld;
    memset( &sw->shapeToWorldNoXlate.rows[3], 0, sizeof( sw->shapeToWorldNoXlate.rows[3] ) );

    for ( int i = 0; i < sw->def->vertexNormalCt; i++ )
    {
        ivec3 refl, reflWorld;
        ivec3 nn;
        ivec3 *vn = &sw->def->vertexNormals[i];
        int32_t dot;

#if 0
        dot = ivec3_dot_scaled( &sw->cameraInShapeSpace, vn );
        ivec3_scale( vn, -dot * 2, &nn );
        ivec3_add( &sw->cameraInShapeSpace, &nn, &refl );
        ivec3_normalize( &refl, &refl );
        ivec3_mtx4_transform( &refl, &sw->shapeToWorldNoXlate, &reflWorld );
#else
        ivec3_mtx4_transform( vn, &sw->shapeToWorldNoXlate, &reflWorld );
#endif

        int32_t mx = reflWorld.x < 0 ? -reflWorld.x : reflWorld.x;
        int32_t my = reflWorld.y < 0 ? -reflWorld.y : reflWorld.y;
        int32_t mz = reflWorld.z < 0 ? -reflWorld.z : reflWorld.z;
        if ( mx > my )
        {
            if ( mx > mz )
            {
                sw->envMapTexCoords[i].u = my << 10;
                sw->envMapTexCoords[i].v = mz << 10;
            }
            else
            {
                sw->envMapTexCoords[i].u = my << 10;
                sw->envMapTexCoords[i].v = mx << 10;
            }
        }
        else
        {
            if ( my > mz )
            {
                sw->envMapTexCoords[i].u = mx << 10;
                sw->envMapTexCoords[i].v = mz << 10;
            }
            else
            {
                sw->envMapTexCoords[i].u = my << 10;
                sw->envMapTexCoords[i].v = mx << 10;
            }
        }
    }
}

void debugPt( int x, int y, int col )
{
    if ( x < 0 || x >= SCR_X || y < 0 || y >= SCR_Y )
        return;
    g_sc.writepage[x + SCR_X * y] = col;
}

void renderTexPolys( shapeWork *sw )
{
    int ct = sw->visPolyCt;
    polyWork **pwp = &sw->sortedPolys[ct - 1];
    while ( ct-- )
    {
        polyWork *pw = *( pwp-- );
        polyDef *pd = pw->def;
        ivec3 *pv = sw->projectedVerts;

        ivec3 *va = &pv[pd->vidx[0]];
        ivec3 *vb = &pv[pd->vidx[1]];
        ivec3 *vc = &pv[pd->vidx[2]];

        TexVert tv[3];
        tv[0].x = va->x;
        tv[0].y = va->y;
        tv[1].x = vb->x;
        tv[1].y = vb->y;
        tv[2].x = vc->x;
        tv[2].y = vc->y;

        for ( int i = 0; i < 3; i++ )
        {
            envMapTexCoord *ev = &sw->envMapTexCoords[pd->vnidx[i]];
            tv[i].u = ev->u;
            tv[i].v = ev->v;
        }

        renderTexTri( &tv[0], &tv[1], &tv[2] );
    }

#if 0
    mtx4            shapeToCameraNoXlate = sw->shapeToCamera;
    memset( &shapeToCameraNoXlate.rows[ 3 ], 0, sizeof( shapeToCameraNoXlate.rows[ 3 ] ) );

    for( int i = 0 ; i < sw->def->polyCt ; i++ )
    {
        polyDef*        p = &sw->def->polys[ i ];
        for( int j = 0 ; j < 3 ; j++ )
        {
            ivec3*          v = &sw->transformedVerts[ p->vidx[ j ] ];
            ivec3*          vn = &sw->def->vertexNormals[ p->vnidx[ j ] ];
            ivec3           vnw;
            ivec3           aa, bb, cc;
            ivec3_mtx4_transform( vn, &shapeToCameraNoXlate, &vnw );

            project_vert( v, &aa );

            ivec3_scale( &vnw, 5000, &vnw );
            ivec3_add( v, &vnw, &bb );
            project_vert( &bb, &cc );

            if( v->z >= NEAR_Z )
            {
                plot_line( aa.x, aa.y, cc.x, cc.y, 0xf0 );
                // debugPt( aa.x, aa.y, 0xf0 );
                // debugPt( cc.x, cc.y, 0x80 );
            }
        }
    }
#endif
}

void renderPolys( shapeWork *sw )
{
    int ct = sw->visPolyCt;
    polyWork **pwp = &sw->sortedPolys[ct - 1];
    while ( ct-- )
    {
        polyWork *pw = *( pwp-- );
        polyDef *pd = pw->def;
        ivec3 *pv = sw->projectedVerts;

        ivec3 *va = &pv[pd->vidx[0]];
        ivec3 *vb = &pv[pd->vidx[1]];
        ivec3 *vc = &pv[pd->vidx[2]];

        renderTri(
            va->x, va->y,
            vb->x, vb->y,
            vc->x, vc->y,
            pw->col );
    }
}

void renderPolysLines( shapeWork *sw )
{
    int ct = sw->visPolyCt;
    polyWork **pwp = &sw->sortedPolys[ct - 1];
    while ( ct-- )
    {
        polyWork *pw = *( pwp-- );
        polyDef *pd = pw->def;
        ivec3 *pv = sw->projectedVerts;

        ivec3 *va = &pv[pd->vidx[0]];
        ivec3 *vb = &pv[pd->vidx[1]];
        ivec3 *vc = &pv[pd->vidx[2]];

        renderTri(
            va->x, va->y,
            vb->x, vb->y,
            vc->x, vc->y,
            pw->col );

        if ( pw->def->attr & 1 )
            plot_line2( va->x, va->y, vb->x, vb->y, sw->lineCol );
        if ( pw->def->attr & 2 )
            plot_line2( vb->x, vb->y, vc->x, vc->y, sw->lineCol );
        if ( pw->def->attr & 4 )
            plot_line2( vc->x, vc->y, va->x, va->y, sw->lineCol );
    }
}

void renderShape( shapeWork *sw )
{
    /*
    -   build matrices etc...
    -   transform and project all points
    -   light all normals
    -   process polys:
            determine visibility, if vis:
                insert into depth band
                calculate lighting color
    -   (ST) flatten vis bands (collect polys into sorted queue for rendering)
    -   render polys
 */

    mtx4_invert( &sw->shapeToWorld, &sw->worldToShapeNoXlate );
    ivec3_set( (ivec3 *) sw->worldToShapeNoXlate.rows[3].v, 0, 0, 0 );

    mtx4_invert( &sw->cameraToWorld, &sw->worldToCamera );
    mtx4_mtx4_mul( &sw->shapeToWorld, &sw->worldToCamera, &sw->shapeToCamera );
    mtx4_invert( &sw->shapeToCamera, &sw->cameraToShape );
    sw->cameraInShapeSpace = *( (ivec3 *) ( &sw->cameraToShape.rows[3].v[0] ) );
    ivec3_mtx4_transform( &sw->lightVec, &sw->worldToShapeNoXlate, &sw->lightInShapeSpace );

    // compute depth band min/max
    int32_t ctrZ = sw->shapeToCamera.rows[3].v[2];
    sw->zmin = ctrZ - sw->def->radius;
    sw->zrange = sw->def->radius * 2;

    // transform and project points
    transformAndProjectPoints( sw );

    // light normals
    processNormals( sw );

    // process polys
    processVis( sw );

    // insert poly ptrs into depth buckets
    for ( int i = 0; i < DEPTH_BAND_CT; i++ )
    {
        sw->depthBands[i].polyCt = 0;
    }
    for ( int i = 0; i < sw->def->polyCt; i++ )
    {
        polyWork *pw = &sw->visPolys[i];
        if ( pw->depth == DEPTH_INVIS )
        {
            continue;
        }
        for ( int j = pw->depth; j < DEPTH_BAND_CT; j++ )
        {
            depthBand *db = &sw->depthBands[j];
            if ( db->polyCt < db->maxPolys )
            {
                db->polyPtrs[db->polyCt++] = pw;
                break;
            }
        }
    }

    //
    // TODO: sort polys within depth bands?
    //

    // build "sorted" list of polys
    polyWork **p = sw->sortedPolys;
    for ( int j = 0; j < DEPTH_BAND_CT; j++ )
    {
        depthBand *db = &sw->depthBands[j];
        for ( int i = 0; i < db->polyCt; i++ )
        {
            *( p++ ) = db->polyPtrs[i];
        }
    }
    sw->visPolyCt = p - sw->sortedPolys;

    // process polys
    if ( sw->flags & SW_FLAG_ENVMAP )
    {
        generateTexCoords( sw );
        renderTexPolys( sw );
    }
    else if ( sw->flags & SW_FLAG_LINES )
    {
        renderPolysLines( sw );
    }
    else
    {
        renderPolys( sw );
    }
}

void print_ivec3( ivec3 *a )
{
    printf( "( %6ld %6ld %6ld )\n", a->x, a->y, a->z );
}

void print_mtx4( mtx4 *b )
{
    for ( int i = 0; i < 4; i++ )
    {
        printf( "[ %6ld %6ld %6ld %6ld ]\n", b->rows[i].v[0], b->rows[i].v[1], b->rows[i].v[2], b->rows[i].v[3] );
    }
    printf( "\n" );
}

void renderShapeSimple( shapeWork *w, int a, int b, int c, int x, int y, int z, int flags )
{
    ivec3 shapePos;
    ivec3_set( &shapePos, x, y, z );
    mtx4_ident( &w->cameraToWorld );
    mtx4_from_angles( a, b, c, &w->shapeToWorld );
    mtx4_set_translate( &w->shapeToWorld, &shapePos );

    w->flags = flags;
    w->lineCol = 255;
    w->forcePolyCol = 240;

    renderShape( w );
}
