#pragma once

#include <vector>

namespace XEngine {

	enum class ShaderDataType {
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		Int2,
		Int3,
		Int4
	};

	struct BufferElement {
		ShaderDataType type;
		uint32_t componentType;
		size_t componentsCount;
		size_t size;
		size_t offset;

		BufferElement(const ShaderDataType type);
	};

	class BufferLayout {
	public:
		BufferLayout(std::initializer_list<BufferElement> elements)
			: curElements(std::move(elements)) {
			size_t offset = 0;
			stride = 0;
			for(auto& element : curElements) {
				element.offset = offset;
				offset += element.size;
				stride += element.size;
			}
		}

		const std::vector<BufferElement>& getElements() const { return curElements; }
		size_t getStride() const { return stride; }
	private:
		std::vector<BufferElement> curElements;
		size_t stride = 0;
	};

	class VertexBuffer {
	public:
		enum class VBUsage {
			Static,
			Dynamic,
			Stream
		};
		VertexBuffer(const void* data, const size_t, BufferLayout bufferLayout, const VBUsage usage = VertexBuffer::VBUsage::Static);
		~VertexBuffer();
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&& vertexBuffer) noexcept;
		VertexBuffer(VertexBuffer&&) noexcept;

		void bind() const;
		static void unbind();
		const BufferLayout& getLayout() const { return layout; }
	private:
		unsigned int curID = 0;
		BufferLayout layout;
	};

}