std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& v) {
	using namespace std;
	copy(v.begin(), v.end(), std::ostream_iterator<std::string>(os, "\n"));
	return os;
}

std::string to_string(const std::vector<std::string>& vec) {
	std::ostringstream oss;
	oss << vec;
	return oss.str();
}

std::string operator+(const std::string& str, const std::vector<std::string>& vec) {
	return str + to_string(vec);
}

std::string operator+(const std::vector<std::string>& vec, const std::string& str) {
	return to_string(vec) + str;
}

namespace RspCrypto {

struct BIODeleter {
	void operator()(BIO* bio) const {
		BIO_free_all(bio);
	}
};

struct X509Deleter {
	void operator()(X509* cert) const {
		X509_free(cert);
	}
};

struct X509_STORE_Deleter {
	void operator()(X509_STORE* store) const {
		X509_STORE_free(store);
	}
};

struct X509_STORE_CTX_Deleter {
	void operator()(X509_STORE_CTX* ctx) const {
		X509_STORE_CTX_free(ctx);
	}
};

struct EVP_PKEY_Deleter {
	void operator()(EVP_PKEY* key) const {
		EVP_PKEY_free(key);
	}
};

struct EVP_MD_CTX_Deleter {
	void operator()(EVP_MD_CTX* ctx) const {
		EVP_MD_CTX_free(ctx);
	}
};

class Base64 {
    public:
	static std::vector<uint8_t> decode(const std::string& b64message) {
		if (b64message.empty()) {
			return {};
		}

		int padding = 0;
		if (b64message.size() > 0) {
			if (b64message[b64message.size() - 1] == '=' && b64message[b64message.size() - 2] == '=') {
				padding = 2;
			} else if (b64message[b64message.size() - 1] == '=') {
				padding = 1;
			}
		}

		size_t decodeLen = (b64message.size() * 3) / 4 - padding;
		std::vector<uint8_t> buffer(decodeLen);

		BIO* b64 = BIO_new(BIO_f_base64());
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // No newlines

		BIO* mem = BIO_new_mem_buf(b64message.c_str(), -1);
		BIO* bioChain = BIO_push(b64, mem);

		std::unique_ptr<BIO, BIODeleter> bioPtr(bioChain);

		int length = BIO_read(bioChain, buffer.data(), b64message.size());
		if (length <= 0) {
			throw std::runtime_error("Failed to decode base64 data");
		}

		buffer.resize(length); // Resize to actual decoded length
		return buffer;
	}

	static std::string encode(const std::vector<uint8_t>& data) {
		if (data.empty()) {
			return {};
		}

		BIO* b64 = BIO_new(BIO_f_base64());
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // No newlines

		BIO* mem = BIO_new(BIO_s_mem());
		BIO* bioChain = BIO_push(b64, mem);

		std::unique_ptr<BIO, BIODeleter> bioPtr(bioChain);

		BIO_write(bioChain, data.data(), data.size());
		BIO_flush(bioChain);

		BUF_MEM* bufferPtr;
		BIO_get_mem_ptr(mem, &bufferPtr);

		return std::string(bufferPtr->data, bufferPtr->length);
	}
};

class HexUtil {
    public:
	static std::vector<uint8_t> hexToBytes(const std::string& hex) {
		if (hex.length() % 2 != 0) {
			throw std::runtime_error("Invalid hex string length");
		}

		std::vector<uint8_t> bytes(hex.length() / 2);
		for (size_t i = 0; i < bytes.size(); ++i) {
			sscanf(hex.c_str() + (i * 2), "%2hhx", &bytes[i]);
		}

		return bytes;
	}

	static std::string bytesToHex(const std::vector<uint8_t>& bytes) {
		return bytesToHex(bytes.data(), bytes.size());
	}

	static std::string bytesToHex(const uint8_t* data, size_t length) {
		std::stringstream ss;
		ss << std::hex << std::uppercase << std::setfill('0');

		for (size_t i = 0; i < length; ++i) {
			ss << std::setw(2) << static_cast<int>(data[i]);
		}

		return ss.str();
	}
};

} // namespace RspCrypto