/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2016, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
copyright notice, this list of conditions and the
following disclaimer.

* Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the
following disclaimer in the documentation and/or other
materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
contributors may be used to endorse or promote products
derived from this software without specific prior
written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/
#include "ModelDiffer.h"
#include <assimp/scene.h>
#include <sstream>

using namespace Assimp;

ModelDiffer::ModelDiffer() {
    // empty
}

ModelDiffer::~ModelDiffer() {
    // empty
}

bool ModelDiffer::isEqual( aiScene *expected, aiScene *toCompare ) {
    if ( expected == toCompare ) {
        return true;
    }

    if ( nullptr == expected ) {
        return false;
    }

    if ( nullptr == toCompare ) {
        return false;
    }

    if ( expected->mNumMeshes != toCompare->mNumMeshes ) {
        std::stringstream stream;
        stream << "Number of meshes not equal ( expected: " << expected->mNumMeshes << ", found : " << toCompare->mNumMeshes << " )\n";
        addDiff( stream.str() );
    }

    for ( unsigned int i = 0; i < expected->mNumMeshes; i++ ) {
        aiMesh *expMesh( expected->mMeshes[ i ] );
        aiMesh *toCompMesh( toCompare->mMeshes[ i ] );
        compareMesh( expMesh, toCompMesh );
    }
}

void ModelDiffer::showReport() {
    if ( m_diffs.empty() ) {
        return;
    }
    
    for ( std::vector<std::string>::iterator it = m_diffs.begin(); it != m_diffs.end(); it++ ) {
        std::cout << *it << "\n";
    }

    std::cout << std::endl;
}

void ModelDiffer::reset() {
    m_diffs.resize( 0 );
}

void ModelDiffer::addDiff( const std::string &diff ) {
    if ( diff.empty() ) {
        return;
    }
    m_diffs.push_back( diff );
}

static std::string dumpVector3( const aiVector3D &toDump ) {
    std::stringstream stream;
    stream << "( " << toDump.x << ", " << toDump.y << ", " << toDump.z << ")";
    return stream.str();
}

static std::string dumpColor4D( const aiColor4D &toDump ) {
    std::stringstream stream;
    stream << "( " << toDump.r << ", " << toDump.g << ", " << toDump.b << ", " << toDump.a << ")";
    return stream.str();
}

bool ModelDiffer::compareMesh( aiMesh *expected, aiMesh *toCompare ) {
    if ( expected == toCompare ) {
        return true;
    }

    if ( nullptr == expected || nullptr == toCompare ) {
        return false;
    }

    if ( expected->mName != toCompare->mName ) {
        std::stringstream stream;
        stream << "Mesh name not equal ( expected: " << expected->mName.C_Str() << ", found : " << toCompare->mName.C_Str() << " )\n";
        addDiff( stream.str() );
    }

    if ( expected->mNumVertices != toCompare->mNumVertices ) {
        std::stringstream stream;
        stream << "Number of vertices not equal ( expected: " << expected->mNumVertices << ", found : " << toCompare->mNumVertices << " )\n";
        addDiff( stream.str() );
        return false;
    }

    // positions
    if ( expected->HasPositions() != toCompare->HasPositions() ) {
        addDiff( "Expected are vertices, toCompare does not have any." );
        return false;
    }

    bool vertEqual( true );
    for ( unsigned int i = 0; i < expected->mNumVertices; i++ ) {
        aiVector3D &expVert( expected->mVertices[ i ] );
        aiVector3D &toCompVert( toCompare->mVertices[ i ] );
        if ( expVert != toCompVert ) {
            std::stringstream stream;
            stream << "Vertex not equal ( expected: " << dumpVector3( expVert ) << ", found: " << dumpVector3( toCompVert ) << "\n";
            addDiff( stream.str() );
            vertEqual = false;
        }
    }
    if ( !vertEqual ) {
        return false;
    }

    // normals
    if ( expected->HasNormals() != toCompare->HasNormals() ) {
        addDiff( "Expected are normals, toCompare does not have any." );
        return false;
    }

    bool normalEqual( true );
    for ( unsigned int i = 0; i < expected->mNumVertices; i++ ) {
        aiVector3D &expNormal( expected->mNormals[ i ] );
        aiVector3D &toCompNormal( toCompare->mNormals[ i ] );
        if ( expNormal != toCompNormal ) {
            std::stringstream stream;
            stream << "Normal not equal ( expected: " << dumpVector3( expNormal ) << ", found: " << dumpVector3( toCompNormal ) << "\n";
            addDiff( stream.str() );
            normalEqual = false;
        }
    }
    if ( !normalEqual ) {
        return false;
    }

    // vertex colors
    bool vertColEqual( true );
    for ( unsigned int a = 0; a < AI_MAX_NUMBER_OF_COLOR_SETS; a++ ) {
        if ( expected->HasVertexColors(a) != toCompare->HasVertexColors(a) ) {
            addDiff( "Expected are normals, toCompare does not have any." );
            return false;
        }
        for ( unsigned int i = 0; i < expected->mNumVertices; i++ ) {
            aiColor4D &expColor4D( expected->mColors[ a ][ i ] );
            aiColor4D &toCompColor4D( toCompare->mColors[ a ][ i ] );
            if ( expColor4D != toCompColor4D ) {
                std::stringstream stream;
                stream << "Color4D not equal ( expected: " << dumpColor4D( expColor4D ) << ", found: " << dumpColor4D( toCompColor4D ) << "\n";
                addDiff( stream.str() );
                vertColEqual = false;
            }
        }
        if ( !vertColEqual ) {
            return false;
        }
    }

    // texture coords
    bool texCoordsEqual( true );
    for ( unsigned int a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; a++ ) {
        if ( expected->HasTextureCoords( a ) != toCompare->HasTextureCoords( a ) ) {
            addDiff( "Expected are texture coords, toCompare does not have any." );
            return false;
        }
        for ( unsigned int i = 0; i < expected->mNumVertices; i++ ) {
            aiVector3D &expTexCoord( expected->mTextureCoords[ a ][ i ] );
            aiVector3D &toCompTexCoord( toCompare->mTextureCoords[ a ][ i ] );
            if ( expTexCoord != toCompTexCoord ) {
                std::stringstream stream;
                stream << "Texture coords not equal ( expected: " << dumpVector3( expTexCoord ) << ", found: " << dumpVector3( toCompTexCoord ) << "\n";
                addDiff( stream.str() );
                vertColEqual = false;
            }
        }
        if ( !vertColEqual ) {
            return false;
        }
    }

    // tangents and bi-tangents
    if ( expected->HasTangentsAndBitangents() != toCompare->HasTangentsAndBitangents() ) {
        addDiff( "Expected are tangents and bi-tangents, toCompare does not have any." );
        return false;
    }
    bool tangentsEqual( true );
    for ( unsigned int i = 0; i < expected->mNumVertices; i++ ) {
        aiVector3D &expTangents( expected->mTangents[ i ] );
        aiVector3D &toCompTangents( toCompare->mTangents[ i ] );
        if ( expTangents != toCompTangents ) {
            std::stringstream stream;
            stream << "Tangents not equal ( expected: " << dumpVector3( expTangents ) << ", found: " << dumpVector3( toCompTangents ) << "\n";
            addDiff( stream.str() );
            tangentsEqual = false;
        }

        aiVector3D &expBiTangents( expected->mBitangents[ i ] );
        aiVector3D &toCompBiTangents( toCompare->mBitangents[ i ] );
        if ( expBiTangents != toCompBiTangents ) {
            std::stringstream stream;
            stream << "Tangents not equal ( expected: " << dumpVector3( expBiTangents ) << ", found: " << dumpVector3( toCompBiTangents ) << "\n";
            addDiff( stream.str() );
            tangentsEqual = false;
        }
    }
    if ( !tangentsEqual ) {
        return false;
    }

    return true;
}
