// OGLVertexStream.cpp
// KlayGE OpenGL顶点数据流类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 初次建立 (2004.4.3)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <algorithm>

#include <KlayGE/OpenGL/OGLVertexStream.hpp>

namespace KlayGE
{
	OGLVertexStream::OGLVertexStream(VertexStreamType type, U8 sizeElement, U8 numElement)
			: VertexStream(type, sizeElement, numElement)
	{
	}

	void OGLVertexStream::Assign(void const * src, size_t numVertices, size_t stride)
	{
		size_t const vertexSize(this->sizeElement() * this->ElementsPerVertex());
		size_t const size(vertexSize * numVertices);

		buffer_.resize(size);

		U8* destPtr(&buffer_[0]);
		U8 const * srcPtr(static_cast<U8 const *>(src));

		if (stride != 0)
		{
			for (size_t i = 0; i < numVertices; ++ i)
			{
				std::copy(srcPtr, srcPtr + vertexSize, destPtr);

				destPtr += vertexSize;
				srcPtr += vertexSize + stride;
			}
		}
		else
		{
			std::copy(srcPtr, srcPtr + size, destPtr);
		}
	}
}
