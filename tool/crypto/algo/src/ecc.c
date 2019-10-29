
#include "ecc.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//CECC()
//{
//  mp_init_multi( &m_Module , &m_CoefA , &m_CoefB , &m_BasepointX , &m_BasepointY , &m_BasepointOrder , NULL );
//  srand( (unsigned)time( NULL ) );
//}
//~CECC()
//{
//  mp_clear_multi( &m_Module , &m_CoefA , &m_CoefB , &m_BasepointX , &m_BasepointY , &m_BasepointOrder , NULL );
//}

mp_int m_Module,m_CoefA,m_CoefB,m_BasepointX,m_BasepointY,m_BasepointOrder;
int m_modLen;

const char cTable[16] =
{
    '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' ,
    '8' , '9' , 'A' , 'B' , 'C' , 'D' , 'E' , 'F'
};
void GenRandStr( char *pRand , int len )
{
    //srand( (unsigned)time( NULL ) );
    while( len -- ) {
        *pRand++ = cTable[rand()&0x0F];
    }
    *pRand = 0;

#ifdef __CERT_PERSONAL__
    srand( (unsigned)time( NULL ) );
#endif
}
void Add2points(mp_int *x1,mp_int *y1,mp_int *x2,mp_int *y2,mp_int *x3,mp_int *y3,mp_int *a,mp_int *p)
{
    mp_int x2x1,y2y1,tempk,temp1;

    mp_init_multi( &x2x1 , &y2y1 , &tempk , &temp1 , NULL );

    if( ( mp_cmp_d( x1 , 0 ) == 0 ) &&
        ( mp_cmp_d( y1 , 0 ) == 0 ) )
    {
        mp_copy( x2 , x3 );
        mp_copy( y2 , y3 );
        goto L;
    }

    if( ( mp_cmp_d( x2 , 0) == 0 ) &&
        ( mp_cmp_d( y2 , 0 ) == 0 ) )
    {
        mp_copy( x1 , x3 );
        mp_copy( y1 , y3 );
        goto L;
    }

    mp_submod( x2 , x1 , p , &x2x1 );
    if( mp_cmp_d( &x2x1 , 0 ) == 0 )
    {
        mp_addmod( y2 , y1 , p , &y2y1 );
        if( mp_cmp_d( &y2y1 , 0 ) == 0 )
        {
            mp_zero( x3 );
            mp_zero( y3 );
            goto L;
        }
        else
        {
            mp_invmod( &y2y1 , p , &tempk );
            mp_mulmod( x1 , x1 , p , &temp1 );
            mp_mul_d( &temp1 , 3 , &temp1 );
            mp_addmod( &temp1 , a , p , &temp1 );
            mp_mulmod( &temp1 , &tempk , p , &tempk );
        }
    }
    else
    {
        mp_invmod( &x2x1 , p , &tempk );
        mp_submod( y2 , y1 , p , &y2y1 );
        mp_mulmod( &y2y1 , &tempk , p , &tempk );
    }

    mp_mulmod( &tempk , &tempk , p , &temp1 );
    mp_submod( &temp1 , x1, p , &temp1 );
    mp_submod( &temp1 , x2 , p , x3 );
    mp_submod( x1 , x3, p , &temp1 );
    mp_mulmod( &temp1 , &tempk, p , &temp1 );
    mp_submod( &temp1 , y1 , p , y3 );
L:
    mp_clear_multi( &x2x1 , &y2y1 , &tempk , &temp1 , NULL );
}

void Mul2points(mp_int *qx,mp_int *qy, mp_int *px, mp_int *py,mp_int *d,mp_int *a,mp_int *p)
{
    mp_int X,Y;
    unsigned int i;
    char Bt_array[800];
    mp_toradix( d , Bt_array , 2 );
    mp_zero( qx );
    mp_zero( qy );
    mp_init( &X );
    mp_init( &Y );
    for( i = 0 ; i < strlen( Bt_array ) ; i ++ )
    {
        Add2points( qx , qy , qx , qy , &X , &Y , a , p );
        mp_copy( &X , qx );
        mp_copy( &Y , qy );
        if( Bt_array[i] == '1' )
        {
            Add2points( qx , qy , px , py , &X , &Y , a , p );
            mp_copy( &X , qx );
            mp_copy( &Y , qy );
        }
    }
    mp_clear( &X );
    mp_clear( &Y );
}
bool CheckKey(mp_int*QX,mp_int*QY,mp_int*sk,mp_int*n,mp_int*A,mp_int*B,mp_int*P)
{
    mp_int tempx,tempy,rx,ry;

    if( ( mp_cmp_d( QX , 0 ) == 0 ) &&
        ( mp_cmp_d( QY , 0 ) == 0 ) )   //Q=O
    {
        return 0;
    }
    if( ( mp_cmp_d( QX , 0 ) == -1 ) ||
        ( mp_cmp( QX , P ) >= 0 ) )     //QX<0||QX>=q
    {
        return 0;
    }
    if( ( mp_cmp_d( QY , 0 ) == -1 ) ||
        ( mp_cmp( QY , P ) >= 0 ) )     //QY<0||QY>=q
    {
        return 0;
    }

    mp_init_multi( &tempx , &tempy , &rx , &ry , NULL );
    mp_expt_d( QX , 3 , &tempx );
    mp_mod( &tempx , P , &tempx );
    /* d = a * b (mod c) */
    //int mp_mulmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);
    mp_mulmod( QX , A , P , &tempy );
    mp_addmod( &tempx , &tempy , P , &tempx );
    mp_addmod( &tempx , B , P , &tempx );
    mp_expt_d( QY , 2 , &tempy );
    mp_mod( &tempy , P , &tempy );
    if( mp_cmp( &tempx , &tempy ) )
    {
        mp_clear_multi( &tempx , &tempy , &rx , &ry , NULL );
        return 0;
    }
    Mul2points( &rx , &ry , QX , QY , n , A , P );  //R=nQ=ndG
    if( ( mp_cmp_d( &rx , 0 ) == 0 ) &&
        ( mp_cmp_d( &ry , 0 ) == 0 ) )                  //R=O
    {
        mp_clear_multi( &tempx , &tempy , &rx , &ry , NULL );
        return 1;
    }
    else
    {
        mp_clear_multi( &tempx , &tempy , &rx , &ry , NULL );
        return 0;
    }
}

bool GenerateKeyPair( char *pPrivateKey , char *pPublicKeyX , char *pPublicKeyY )
{
    mp_int nm1,d,Qx,Qy;
    char cD[MAX_CHAR_LEN];
    bool isKeyGen;

    mp_init_multi( &nm1 , &d , &Qx , &Qy , NULL );
    mp_sub_d( &m_BasepointOrder , 0x01 , &nm1 );

    do
    {
        GenRandStr( cD , m_modLen );
        mp_read_radix( &d , cD , 16 );
    } while( mp_cmp( &d , &nm1 ) != -1 );

    isKeyGen = 0;
    while( mp_cmp( &d , &nm1 ) == -1 )
    {
        Mul2points( &Qx , &Qy , &m_BasepointX , &m_BasepointY , &d , &m_CoefA , &m_Module );
        if( CheckKey( &Qx , &Qy , &d , &m_BasepointOrder , &m_CoefA , &m_CoefB , &m_Module ) )
        {
            isKeyGen = 1;
            break;
        }
        else
        {
            mp_add_d( &d , 0x01 , &d );
        }
    }
    if( isKeyGen )
    {
        mp_toradix( &d , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( pPrivateKey , '0' , m_modLen - strlen(cD) );
            pPrivateKey += ( m_modLen - strlen(cD) );
        }
        strcpy( pPrivateKey , cD );
        mp_toradix( &Qx , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( pPublicKeyX , '0' , m_modLen - strlen(cD) );
            pPublicKeyX += ( m_modLen - strlen(cD) );
        }
        strcpy( pPublicKeyX , cD );
        mp_toradix( &Qy , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( pPublicKeyY , '0' , m_modLen - strlen(cD) );
            pPublicKeyY += ( m_modLen - strlen(cD) );
        }
        strcpy( pPublicKeyY , cD );
    }
    else
    {
        mp_set( &d , 0 );
        memset( pPrivateKey , '0' , m_modLen );
        pPrivateKey[m_modLen] = 0x00;
        memset( pPublicKeyX , '0' , m_modLen );
        pPublicKeyX[m_modLen] = 0x00;
        memset( pPublicKeyY , '0' , m_modLen );
        pPublicKeyY[m_modLen] = 0x00;
    }
    mp_clear_multi( &nm1 , &d , &Qx , &Qy , NULL );
    return isKeyGen;
}
bool GeneratePublicKey( char *pPrivateKey , char *pPublicKeyX , char *pPublicKeyY )
{
    mp_int nm1,d,Qx,Qy;
    char cD[MAX_CHAR_LEN];
    bool isKeyGen;

    mp_init_multi( &nm1 , &d , &Qx , &Qy , NULL );
    mp_sub_d( &m_BasepointOrder , 0x01 , &nm1 );

    mp_read_radix( &d , pPrivateKey , 16 );
    if( mp_cmp( &d , &nm1 ) != -1 )
    {
        mp_clear_multi( &nm1 , &d , &Qx , &Qy , NULL );
        return 0;
    }

    isKeyGen = 0;
    while( mp_cmp( &d , &nm1 ) == -1 )
    {
        Mul2points( &Qx , &Qy , &m_BasepointX , &m_BasepointY , &d , &m_CoefA , &m_Module );
        if( CheckKey( &Qx , &Qy , &d , &m_BasepointOrder , &m_CoefA , &m_CoefB , &m_Module ) )
        {
            isKeyGen = 1;
            break;
        }
        else
        {
            mp_add_d( &d , 0x01 , &d );
        }
    }
    if( isKeyGen )
    {
        mp_toradix( &d , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( pPrivateKey , '0' , m_modLen - strlen(cD) );
            pPrivateKey += ( m_modLen - strlen(cD) );
        }
        strcpy( pPrivateKey , cD );
        mp_toradix( &Qx , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( pPublicKeyX , '0' , m_modLen - strlen(cD) );
            pPublicKeyX += ( m_modLen - strlen(cD) );
        }
        strcpy( pPublicKeyX , cD );
        mp_toradix( &Qy , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( pPublicKeyY , '0' , m_modLen - strlen(cD) );
            pPublicKeyY += ( m_modLen - strlen(cD) );
        }
        strcpy( pPublicKeyY , cD );
    }
    else
    {
        mp_set( &d , 0 );
        memset( pPrivateKey , '0' , m_modLen );
        pPrivateKey[m_modLen] = 0x00;
        memset( pPublicKeyX , '0' , m_modLen );
        pPublicKeyX[m_modLen] = 0x00;
        memset( pPublicKeyY , '0' , m_modLen );
        pPublicKeyY[m_modLen] = 0x00;
    }
    mp_clear_multi( &nm1 , &d , &Qx , &Qy , NULL );
    return isKeyGen;
}
bool Sign( char *pHash , char *pk , char *pPrivateKey , char *pr , char *ps )
{
    mp_int k,d,kinv,Hash,Qx,Qy,r,s;
    char cD[MAX_CHAR_LEN];
    char ppk[MAX_CHAR_LEN];

    pr[0] = 0;
    ps[0] = 0;

    if( ( strlen( pHash) == 0 ) ||
        ( strlen( pPrivateKey) == 0 ) )
    {
        return 0;
    }

    mp_init_multi( &k  , &d , &kinv , &Hash , &Qx , &Qy , &r , &s , NULL );

    mp_read_radix( &Hash , pHash , 16 );
    mp_mod( &Hash , &m_BasepointOrder , &Hash );
    if( pk == NULL )
    {
        pk = ppk;
        GenRandStr( pk , m_modLen );
    }
    mp_read_radix( &d , pPrivateKey , 16 );
    mp_read_radix( &k , pk , 16 );

    while( 1 )
    {
        // 1. k = RNG
        mp_mod( &k , &m_BasepointOrder , &k );
        if( mp_cmp_d( &k , 0 ) == 0 )
        {
            mp_add_d( &k , 1 , &k );
        }
        // 2. Q = [k]G
        Mul2points( &Qx , &Qy , &m_BasepointX , &m_BasepointY , &k , &m_CoefA , &m_Module );

        // 3. r = Qx mod n
        mp_mod( &Qx , &m_BasepointOrder , &r );
        if( mp_cmp_d( &r , 0 ) == 0 )
        {
            continue;
        }

        // 4. kinv = k(-1) mod n
        mp_invmod( &k , &m_BasepointOrder , &kinv );
        // 5. s = kinv*( r*d + H(M)) mod n
        mp_mulmod( &r , &d , &m_BasepointOrder , &s );
        mp_addmod( &Hash , &s , &m_BasepointOrder , &s );
        mp_mulmod( &kinv , &s, &m_BasepointOrder , &s );
        // if s = 0 goto 1.
        if( mp_cmp_d( &s , 0 ) == 0 )
        {
            continue;
        }

        // Out r
        mp_toradix( &r , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( pr , '0' , m_modLen - strlen(cD) );
            pr += ( m_modLen - strlen(cD) );
        }
        strcpy( pr , cD );
        // Out s
        mp_toradix( &s , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( ps , '0' , m_modLen - strlen(cD) );
            ps += ( m_modLen - strlen(cD) );
        }
        strcpy( ps , cD );
        mp_clear_multi( &k  , &d , &kinv , &Hash , &Qx , &Qy , &r , &s , NULL );
        return 1;
    }
}
bool Verify( char *pHash , char *pPublicKeyX , char *pPublicKeyY , char *pr , char *ps )
{
    mp_int Hash,Qx,Qy,Px,Py,tempux,tempuy,r,s,sinv,uu1,uu2,v;
    bool ret = 0;

    mp_init_multi( &Hash , &tempux , &tempuy , &Qx , &Qy , &Px , &Py , &r , &s , &sinv , &uu1 , &uu2 , &v , NULL );

    mp_read_radix( &Hash , pHash , 16 );
    mp_mod( &Hash , &m_BasepointOrder , &Hash );
    mp_read_radix( &Px , pPublicKeyX , 16 );
    mp_read_radix( &Py , pPublicKeyY , 16 );
    mp_read_radix( &r , pr , 16 );
    mp_read_radix( &s , ps , 16 );

    // 1. verify r,s
    if( ( mp_cmp_d( &s , 0 ) == 0 ) ||
        ( mp_cmp( &s , &m_BasepointOrder ) > -1 ) ||
        ( mp_cmp_d( &r , 0 ) == 0 ) ||
        ( mp_cmp( &r , &m_BasepointOrder ) > -1 ) )
    {
        mp_clear_multi( &Hash , &tempux , &tempuy , &Qx , &Qy , &Px , &Py , &r , &s , &sinv , &uu1 , &uu2 , &v , NULL );
        return 0;
    }

    // 2. sinv = s(-1) mod n
    mp_invmod( &s , &m_BasepointOrder , &sinv );
    // 3. u1 = sinv * H(M) mod n;u2 = sinv * r mod n
    mp_mulmod( &sinv , &Hash , &m_BasepointOrder , &uu1 );
    mp_mulmod( &sinv , &r , &m_BasepointOrder , &uu2 );
    // 4. Q = [u1]G + [u2]PA
    Mul2points( &tempux , &tempuy , &m_BasepointX , &m_BasepointY , &uu1 , &m_CoefA , &m_Module );
    Mul2points( &Qx , &Qy , &Px , &Py , &uu2 , &m_CoefA , &m_Module );
    Add2points( &tempux , &tempuy , &Qx , &Qy , &Qx , &Qy , &m_CoefA , &m_Module );
    // If Q = O, output Error and terminate
    if( ( mp_cmp_d( &Qx , 0 ) == 0 ) &&
        ( mp_cmp_d( &Qy , 0 ) == 0 ) )
    {
        mp_clear_multi( &Hash , &tempux , &tempuy , &Qx , &Qy , &Px , &Py , &r , &s , &sinv , &uu1 , &uu2 , &v , NULL );
        return 0;
    }
    // 5. v = Qx mod n
    mp_mod( &Qx , &m_BasepointOrder , &v );
    // 6. Output True if v = r, and False otherwise
    if( mp_cmp( &r , &v ) == 0 )
    {
        ret = 1;
    }

    mp_clear_multi( &Hash , &tempux , &tempuy , &Qx , &Qy , &Px , &Py , &r , &s , &sinv , &uu1 , &uu2 , &v , NULL );
    return ret;
}
bool KeyAgreement( char *pdA , char *pPBx , char *pPBy , char *pZab )
{
    mp_int d,Px,Py,Qx,Qy,Sx,Sy,h,l;
    char cD[MAX_CHAR_LEN];

    if( ( strlen( pdA ) == 0 ) ||
        ( strlen( pPBx ) == 0 ) ||
        ( strlen( pPBy ) == 0 ) )
    {
        return 0;
    }

    mp_init_multi( &d , &Px , &Py , &Qx , &Qy , &Sx , &Sy , &h , &l , NULL );

    mp_read_radix( &d , pdA , 16 );
    mp_read_radix( &Px , pPBx , 16 );
    mp_read_radix( &Py , pPBy , 16 );

    *pZab = 0;

    // 1. l = h(-1) mod n
    mp_set( &h , 1 );
    mp_invmod( &h , &m_BasepointOrder , &l );

    // 2. Q = [h] * P
    Mul2points( &Qx , &Qy , &Px , &Py , &h , &m_CoefA , &m_Module );

    // 3. SAB = [ d * l mod n ] * Q
    mp_mulmod( &l , &d , &m_BasepointOrder , &l );
    Mul2points( &Sx , &Sy , &Qx , &Qy , &l , &m_CoefA , &m_Module );
    // If SAB = O, output Error and terminate
    if( ( mp_cmp_d( &Sx , 0 ) == 0 ) &&
        ( mp_cmp_d( &Sy , 0 ) == 0 ) )
    {
        mp_clear_multi( &d , &Px , &Py , &Qx , &Qy , &Sx , &Sy , &h , &l , NULL );
        return 0;
    }

    // Out z
    mp_toradix( &Sx , cD , 16 );
    if( (int)strlen(cD) & 0x01 )
    {
        *pZab++ = '0';
    }
    strcpy( pZab , cD );

    mp_clear_multi( &d , &Px , &Py , &Qx , &Qy , &Sx , &Sy , &h , &l , NULL );
    return 1;
}
bool SetCurveParam( char *pModule , char *pCoefA , char *pCoefB , char *pBasepointX , char *pBasepointY , char *pBasepointOrder )
{
    //int ret;
    mp_read_radix( &m_Module , pModule , 16 );
    mp_read_radix( &m_CoefA , pCoefA , 16 );
    mp_read_radix( &m_CoefB , pCoefB , 16 );
    mp_read_radix( &m_BasepointX , pBasepointX , 16 );
    mp_read_radix( &m_BasepointY , pBasepointY , 16 );
    mp_read_radix( &m_BasepointOrder , pBasepointOrder , 16 );
    m_modLen = strlen( pBasepointOrder );
    return CheckCurveParameter();
}
bool Encrypt( char *pPlain , char *pPublicKeyX , char *pPublicKeyY , char *pk , char *pCm , char *px1 , char *py1 )
{
    mp_int PlainText,PublicKeyX,PublicKeyY,k,Cm,x1,y1,tpx,tpy;
    char cD[MAX_CHAR_LEN],ppk[MAX_CHAR_LEN];

    mp_init_multi( &PlainText , &PublicKeyX , &PublicKeyY , &k , &Cm , &x1 , &y1 , &tpx , &tpy , NULL );

    if( pk == NULL )
    {
        pk = ppk;
        GenRandStr( pk , m_modLen );
    }

    mp_read_radix( &PlainText , pPlain , 16 );
    mp_read_radix( &PublicKeyX , pPublicKeyX , 16 );
    mp_read_radix( &PublicKeyY , pPublicKeyY , 16 );
    mp_read_radix( &k , pk , 16 );

    mp_mod( &PlainText , &m_Module , &PlainText );

    while( 1 )
    {
        // 生成一个临时私钥
        mp_mod( &k , &m_BasepointOrder , &k );
        if( mp_cmp_d( &k , 0 ) == 0 )
        {
            mp_add_d( &k , 1 , &k );
        }
        // 根据临时私钥计算临时公钥
        Mul2points( &x1 , &y1 , &m_BasepointX , &m_BasepointY , &k , &m_CoefA , &m_Module );
        Mul2points( &tpx , &tpy , &PublicKeyX , &PublicKeyY , &k , &m_CoefA , &m_Module );
        if( mp_cmp_d( &tpx , 0 ) == 0 )
        {
            continue;
        }
        mp_mulmod( &PlainText , &tpx , &m_Module , &Cm );

        // Out Cm
        mp_toradix( &Cm , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( pCm , '0' , m_modLen - strlen(cD) );
            pCm += ( m_modLen - strlen(cD) );
        }
        strcpy( pCm , cD );
        // Out x1
        mp_toradix( &x1 , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( px1 , '0' , m_modLen - strlen(cD) );
            px1 += ( m_modLen - strlen(cD) );
        }
        strcpy( px1 , cD );
        // Out y1
        mp_toradix( &y1 , cD , 16 );
        if( (int)strlen(cD) < m_modLen )
        {
            memset( py1 , '0' , m_modLen - strlen(cD) );
            py1 += ( m_modLen - strlen(cD) );
        }
        strcpy( py1 , cD );
        break;
    }

    mp_clear_multi( &PlainText , &PublicKeyX , &PublicKeyY , &k , &Cm , &x1 , &y1 , &tpx , &tpy , NULL );
    return 1;
}

bool Decrypt( char *pPrivateKey , char *pCm , char *px1 , char *py1 , char *pPlain )
{
    mp_int PrivateKey,Cm,x1,y1,Plain,tempx,tempy;

    mp_init_multi( &PrivateKey , &Cm , &x1 , &y1 , &Plain , &tempx , &tempy , NULL );

    mp_read_radix( &PrivateKey , pPrivateKey , 16 );
    mp_read_radix( &Cm , pCm , 16 );
    mp_read_radix( &x1 , px1 , 16 );
    mp_read_radix( &y1 , py1 , 16 );

    mp_expt_d( &x1 , 3 , &tempx );
    mp_mod( &tempx , &m_Module , &tempx );

    mp_mulmod( &x1 , &m_CoefA , &m_Module , &tempy );
    mp_addmod( &tempx , &tempy , &m_Module , &tempx );
    mp_addmod( &tempx , &m_CoefB , &m_Module , &tempx );
    mp_expt_d( &y1 , 2 , &tempy );
    mp_mod( &tempy , &m_Module , &tempy );
    if( mp_cmp_d( &tempy , 0 ) == 0 )
    {
        mp_clear_multi( &PrivateKey , &Cm , &x1 , &y1 , &Plain , &tempx , &tempy , NULL );
        return 0;
    }

    Mul2points( &tempx , &tempy , &x1 , &y1 , &PrivateKey , &m_CoefA , &m_Module );
    if( mp_cmp_d( &tempx , 0 ) == 0 )
    {
        mp_clear_multi( &PrivateKey , &Cm , &x1 , &y1 , &Plain , &tempx , &tempy , NULL );
        return 0;
    }

    mp_invmod( &tempx , &m_Module , &tempx );
    mp_mulmod( &Cm , &tempx , &m_Module , &Plain );

    // Out Plaintext
    mp_toradix( &Plain , pPlain , 16 );

    mp_clear_multi( &PrivateKey , &Cm , &x1 , &y1 , &Plain , &tempx , &tempy , NULL );
    return 1;
}
bool CheckCurveParameter( void )
{
    mp_int tpy,tpx,tpz;

    mp_init_multi( &tpx , &tpy , &tpz ,  NULL );

    //y^2 ≡ x^3 + ax + b (mod p)
    mp_expt_d( &m_BasepointY , 2 ,&tpy );
    mp_mod( &tpy , &m_Module , &tpy );                          // tpy = ( y ^ 2 ) mod n

    mp_expt_d( &m_BasepointX , 3 ,&tpx );
    mp_mod( &tpx , &m_Module , &tpx );                          // tpx = ( x ^ 3 ) mod n
    mp_mulmod( &m_CoefA , &m_BasepointX , &m_Module , &tpz );   // tpz = ( a * x ) mod n
    mp_addmod( &tpz , &m_CoefB , &m_Module , &tpz );            // tpz = ( a * x + b ) mod n
    mp_addmod( &tpz , &tpx , &m_Module , &tpx );                // tpx = ( x ^ 3 + a * x + b ) mod n
    if( mp_cmp( &tpx , &tpy ) != 0 )                            // tpx != tpz
    {
        mp_clear_multi( &tpx , &tpy , &tpz ,  NULL );
        return 0;
    }

    // n!=0
    if( mp_cmp_d( &m_BasepointOrder , 0 ) == 0 )
    {
        mp_clear_multi( &tpx , &tpy , &tpz ,  NULL );
        return 0;
    }

    // ndG ≡ 0;假定d=1，即n * ( Gx , Gy ) ≡ 0
    Mul2points( &tpx , &tpy , &m_BasepointX , &m_BasepointY , &m_BasepointOrder , &m_CoefA , &m_Module );

    if( ( mp_cmp_d( &tpx , 0 ) != 0 ) ||
        ( mp_cmp_d( &tpy , 0 ) != 0 ) )
    {
        mp_clear_multi( &tpx , &tpy , &tpz ,  NULL );
        return 0;
    }

    // 4a^3+27b^2 ≠0
    mp_expt_d( &m_CoefA , 3 , &tpx );               // tpx = a ^ 3
    mp_mul_d( &tpx , 4 , &tpx );                    // tpx = 4 * ( a ^ 3 )
    mp_expt_d( &m_CoefB , 2 , &tpy );               // tpy = b ^ 2
    mp_mul_d( &tpy , 27 , &tpy );                   // tpy = 27 * ( b ^ 2 )
    mp_addmod( &tpx , &tpy , &m_Module , &tpz );    // tpz = ( 4 * ( a ^ 3 ) + 27 * ( b ^ 2 ) ) mod n
    if( mp_cmp_d( &tpz , 0 ) == 0 )
    {
        mp_clear_multi( &tpx , &tpy , &tpz ,  NULL );
        return 0;
    }

    mp_clear_multi( &tpx , &tpy , &tpz ,  NULL );

    return 1;
}

bool CheckPublicKey( char * pPublicKeyx , char * pPublicKeyy )
{
    mp_int PublicKeyx,PublicKeyy,tpy,tpx,tpz;

    mp_init_multi( &tpx , &tpy , &tpz , &PublicKeyx , &PublicKeyy , NULL );

    mp_read_radix( &PublicKeyx , pPublicKeyx , 16 );
    mp_read_radix( &PublicKeyy , pPublicKeyy , 16 );

    // PublicKey != O
    if( ( mp_cmp_d( &PublicKeyx , 0 ) == 0 ) &&
        ( mp_cmp_d( &PublicKeyy , 0 ) == 0 ) )
    {
        mp_clear_multi( &tpx , &tpy , &tpz , &PublicKeyx , &PublicKeyy , NULL );
        return 0;
    }
    // PublicKeyx >= 0 && PublicKeyx < Moudle
    // PublicKeyy >= 0 && PublicKeyy < Moudle
    if( ( mp_cmp_d( &PublicKeyx , 0 ) == -1 ) ||
        ( mp_cmp( &PublicKeyx , &m_Module ) >= 0 ) ||
        ( mp_cmp_d( &PublicKeyy , 0 ) == -1 ) ||
        ( mp_cmp( &PublicKeyy , &m_Module ) >= 0 ) )
    {
        mp_clear_multi( &tpx , &tpy , &tpz , &PublicKeyx , &PublicKeyy , NULL );
        return 0;
    }

    //y^2 ≡ x^3 + ax + b (mod p)
    mp_expt_d( &PublicKeyy , 2 ,&tpy );
    mp_mod( &tpy , &m_Module , &tpy );                          // tpy = ( y ^ 2 ) mod n

    mp_expt_d( &PublicKeyx , 3 ,&tpx );
    mp_mod( &tpx , &m_Module , &tpx );                          // tpx = ( x ^ 3 ) mod n
    mp_mulmod( &m_CoefA , &PublicKeyx , &m_Module , &tpz );     // tpz = ( a * x ) mod n
    mp_addmod( &tpz , &m_CoefB , &m_Module , &tpz );            // tpz = ( a * x + b ) mod n
    mp_addmod( &tpz , &tpx , &m_Module , &tpx );                // tpx = ( x ^ 3 + a * x + b ) mod n
    if( mp_cmp( &tpx , &tpy ) != 0 )                            // tpx != tpz
    {
        mp_clear_multi( &tpx , &tpy , &tpz , &PublicKeyx , &PublicKeyy , NULL );
        return 0;
    }

    // nQ ≡ 0,即n * (Qx,Qy) ≡ 0
    Mul2points( &tpx , &tpy , &PublicKeyx , &PublicKeyy , &m_BasepointOrder , &m_CoefA , &m_Module );

    if( ( mp_cmp_d( &tpx , 0 ) != 0 ) ||
        ( mp_cmp_d( &tpy , 0 ) != 0 ) )
    {
        mp_clear_multi( &tpx , &tpy , &tpz , &PublicKeyx , &PublicKeyy , NULL );
        return 0;
    }
    mp_clear_multi( &tpx , &tpy , &tpz , &PublicKeyx , &PublicKeyy , NULL );
    return 1;
}

bool CheckPrivateKey( char *pPrivateKey )
{
    mp_int PrivateKey;

    mp_init( &PrivateKey );

    mp_read_radix( &PrivateKey , pPrivateKey , 16 );

    // PrivateKey > 0 && PrivateKey < BasepointOrder
    if( ( mp_cmp_d( &PrivateKey , 0 ) <= 0 ) ||
        ( mp_cmp( &PrivateKey , &m_BasepointOrder ) > 0 ) )
    {
        mp_clear( &PrivateKey );
        return 0;
    }

    mp_clear( &PrivateKey );

    return 1;
}
