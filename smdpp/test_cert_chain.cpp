/* ============================================================================
 * STANDALONE TEST TOOL: Certificate Chain Verification
 * Tests the verifyCertificateChainDynamic function with SGP.22 certificates
 * ============================================================================ */

#include <iostream>
#include <vector>
#include <memory>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <functional>
#include "smdvalcpp2.cpp"

using namespace RspCrypto;
namespace fs = std::filesystem;

// Helper function to print certificate info
void printCertInfo(const std::string& label, X509* cert) {
    if (!cert) {
        std::cout << label << ": NULL" << std::endl;
        return;
    }

    std::string subject = CertificateUtil::getSubjectName(cert);
    std::string issuer = CertificateUtil::getIssuerName(cert);
    auto ski = CertificateUtil::getSubjectKeyIdentifier(cert);
    auto aki = CertificateUtil::getAuthorityKeyIdentifier(cert);

    std::cout << label << ":" << std::endl;
    std::cout << "  Subject: " << subject << std::endl;
    std::cout << "  Issuer:  " << issuer << std::endl;
    std::cout << "  SKI:     " << HexUtil::bytesToHex(ski) << std::endl;
    std::cout << "  AKI:     " << HexUtil::bytesToHex(aki) << std::endl;

    // Check if self-signed
    if (X509_check_issued(cert, cert) == X509_V_OK) {
        std::cout << "  Type:    Self-signed (potential root)" << std::endl;
    }
    std::cout << std::endl;
}

// Get short cert identifier with SKI
std::string getCertId(X509* cert) {
    if (!cert) return "NULL";

    std::string subject = CertificateUtil::getSubjectName(cert);
    auto ski = CertificateUtil::getSubjectKeyIdentifier(cert);

    // For readability, just show first 8 hex chars of SKI
    std::string skiShort = HexUtil::bytesToHex(ski);
    if (skiShort.length() > 8) {
        skiShort = skiShort.substr(0, 8) + "...";
    }

    return subject + " (SKI: " + skiShort + ")";
}

// Load certificates from a directory and categorize them
struct CertificateSet {
    std::vector<std::unique_ptr<X509, X509Deleter>> roots;
    std::vector<std::unique_ptr<X509, X509Deleter>> intermediates;
    std::unique_ptr<X509, X509Deleter> euicc;
};

// Helper function to load certificates from a directory with filtering
std::vector<std::unique_ptr<X509, X509Deleter>> loadCertsFromDirectory(
    const std::string& dirPath,
    const std::string& dirLabel,
    const std::string& filenamePrefix = "",
    std::function<bool(const std::string&)> additionalFilter = nullptr) {

    std::vector<std::unique_ptr<X509, X509Deleter>> result;
    std::set<std::string> loadedSKIs;  // Track loaded certificates by SKI
    std::unordered_map<std::string, std::vector<fs::path>> certFiles;  // Group files by base name

    if (!fs::exists(dirPath)) {
        std::cout << dirLabel << " directory does not exist: " << dirPath << std::endl;
        return result;
    }

    std::cout << "Loading " << dirLabel << " from: " << dirPath << std::endl;

    // First pass: collect all certificate files
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        auto ext = entry.path().extension().string();
        auto filename = entry.path().filename().string();
        auto stem = entry.path().stem().string();

        // Check file extension
        if (ext != ".der" && ext != ".pem") {
            continue;
        }

        // Skip private key files (SK_ prefix)
        if (filename.find("SK_") == 0) {
            continue;
        }

        // Check filename prefix if specified
        if (!filenamePrefix.empty() && filename.find(filenamePrefix) != 0) {
            continue;
        }

        // Apply additional filter if provided
        if (additionalFilter && !additionalFilter(filename)) {
            continue;
        }

        certFiles[stem].push_back(entry.path());
    }

    // Second pass: load certificates, preferring .der over .pem
    for (const auto& [stem, paths] : certFiles) {
        // Sort paths to prefer .der files
        auto sortedPaths = paths;
        std::sort(sortedPaths.begin(), sortedPaths.end(), [](const fs::path& a, const fs::path& b) {
            // .der comes before .pem
            return a.extension() == ".der" && b.extension() != ".der";
        });

        // Try to load from the first (preferred) path
        for (const auto& path : sortedPaths) {
            try {
                auto certs = CertificateUtil::loadCertificateChain(path.string());
                bool loaded = false;
                for (auto& cert : certs) {
                    if (cert) {
                        auto ski = CertificateUtil::getSubjectKeyIdentifier(cert.get());
                        std::string skiHex = HexUtil::bytesToHex(ski);

                        // Check if we've already loaded a cert with this SKI
                        if (loadedSKIs.find(skiHex) == loadedSKIs.end()) {
                            std::cout << "  Found certificate: " << getCertId(cert.get())
                                      << " from " << path.filename() << std::endl;
                            loadedSKIs.insert(skiHex);
                            result.push_back(std::move(cert));
                            loaded = true;
                        } else {
                            std::cout << "  Skipping duplicate: " << getCertId(cert.get())
                                      << " from " << path.filename() << std::endl;
                        }
                    }
                }
                if (loaded) {
                    break;  // Don't try other formats if we successfully loaded from this one
                }
            } catch (const std::exception& e) {
                std::cerr << "  Error loading " << path << ": " << e.what() << std::endl;
            }
        }
    }

    return result;
}

CertificateSet loadAndCategorizeCerts(const std::string& baseDir, const std::string& filter = "") {
    CertificateSet certSet;
    std::set<std::string> rootSubjects;

    // Load CI root certificates (NIST and BRP)
    std::function<bool(const std::string&)> filterFunc = nullptr;
    if (!filter.empty()) {
        filterFunc = [&filter](const std::string& filename) {
            return filename.find(filter) != std::string::npos;
        };
    }

    // First load eUICC certificate to know what to exclude from intermediates
    auto euiccCerts = loadCertsFromDirectory(
        baseDir + "/eUICC",
        "eUICC certificates",
        "CERT_EUICC",  // Specific prefix for eUICC certs
        filterFunc
    );

    // Use the first eUICC certificate found
    if (!euiccCerts.empty() && euiccCerts[0]) {
        certSet.euicc = std::move(euiccCerts[0]);
        std::cout << "\nTarget eUICC certificate: " << getCertId(certSet.euicc.get()) << std::endl;
    } else {
        std::cout << "\nERROR: No eUICC certificate found" << std::endl;
        return certSet;
    }

    auto ciCerts = loadCertsFromDirectory(
        baseDir + "/CertificateIssuer",
        "CI certificates",
        "",  // No prefix filter
        filterFunc
    );

    // Identify and store root certificates
    for (auto& cert : ciCerts) {
        if (cert && X509_check_issued(cert.get(), cert.get()) == X509_V_OK) {
            // Ensure this is not the target certificate
            if (X509_cmp(cert.get(), certSet.euicc.get()) == 0) {
                std::cout << "  ERROR: Target certificate found in root certificates" << std::endl;
                continue;
            }
            std::string subject = CertificateUtil::getSubjectName(cert.get());
            rootSubjects.insert(subject);
            std::cout << "  Identified as root: " << getCertId(cert.get()) << std::endl;
            certSet.roots.push_back(std::move(cert));
        }
    }

    // Load EUM certificates (intermediates)
    auto eumCerts = loadCertsFromDirectory(
        baseDir + "/EUM",
        "EUM certificates",
        "CERT_",  // Only load certificate files, not private keys
        filterFunc
    );

    // Store as intermediates (excluding any that are roots or the target)
    std::cout << "\nProcessing EUM certificates as intermediates:" << std::endl;
    for (auto& cert : eumCerts) {
        if (cert) {
            // Ensure this is not the target certificate
            if (X509_cmp(cert.get(), certSet.euicc.get()) == 0) {
                std::cout << "  Skipped (is the target eUICC): " << getCertId(cert.get()) << std::endl;
                continue;
            }
            std::string subject = CertificateUtil::getSubjectName(cert.get());
            if (rootSubjects.find(subject) == rootSubjects.end()) {
                std::cout << "  Added as intermediate: " << getCertId(cert.get()) << std::endl;
                certSet.intermediates.push_back(std::move(cert));
            } else {
                std::cout << "  Skipped (is a root): " << getCertId(cert.get()) << std::endl;
            }
        }
    }

    return certSet;
}

// Test certificate chain verification
bool testChainVerification(const CertificateSet& certSet, bool verbose = true) {
    if (!certSet.euicc) {
        std::cerr << "ERROR: No eUICC certificate loaded!" << std::endl;
        return false;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "Testing Certificate Chain Verification" << std::endl;
    std::cout << "========================================" << std::endl;

    // Print certificate details
    if (verbose) {
        std::cout << "\nCertificate Details:" << std::endl;
        printCertInfo("Target Certificate (eUICC)", certSet.euicc.get());

        std::cout << "Intermediate Certificates (" << certSet.intermediates.size() << "):" << std::endl;
        for (size_t i = 0; i < certSet.intermediates.size(); i++) {
            printCertInfo("  Intermediate " + std::to_string(i+1), certSet.intermediates[i].get());
        }

        std::cout << "Root Certificates (" << certSet.roots.size() << "):" << std::endl;
        for (size_t i = 0; i < certSet.roots.size(); i++) {
            printCertInfo("  Root " + std::to_string(i+1), certSet.roots[i].get());
        }
    }

    // Test each root certificate
    bool anySuccess = false;
    for (size_t i = 0; i < certSet.roots.size(); i++) {
        std::cout << "\n--- Testing with root " << (i+1) << " ---" << std::endl;
        std::cout << "Root: " << getCertId(certSet.roots[i].get()) << std::endl;

        // Build untrusted chain (intermediates only, not eUICC)
        std::vector<X509*> untrustedChain;
        for (const auto& intermediate : certSet.intermediates) {
            untrustedChain.push_back(intermediate.get());
        }

        std::cout << "Untrusted chain size: " << untrustedChain.size() << " certificates" << std::endl;

        // Verify the chain
        try {
            bool result = CertificateUtil::verifyCertificateChainDynamic(
                certSet.euicc.get(),           // Target certificate
                untrustedChain,                // Untrusted intermediates
                certSet.roots[i].get(),        // Trusted root
                verbose                        // Verbose output
            );

            if (result) {
                std::cout << "Chain verification SUCCESSFUL with this root" << std::endl;
                anySuccess = true;
            } else {
                std::cout << "Chain verification FAILED with this root" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Chain verification ERROR: " << e.what() << std::endl;
        }
    }

    return anySuccess;
}

// Test with superfluous certificates
void testWithSuperfluousCerts(const CertificateSet& certSet, const std::string& baseDir, const std::string& filter = "") {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Testing With Superfluous Certificates" << std::endl;
    std::cout << "========================================" << std::endl;

    if (!certSet.euicc || certSet.roots.empty()) {
        std::cerr << "ERROR: Missing required certificates!" << std::endl;
        return;
    }

    // Debug: Check what we have in certSet at the start
    std::cout << "\nDEBUG: CertSet contents at start of superfluous test:" << std::endl;
    std::cout << "  eUICC: " << (certSet.euicc ? getCertId(certSet.euicc.get()) : "NULL") << std::endl;
    std::cout << "  Intermediates (" << certSet.intermediates.size() << "):" << std::endl;
    for (size_t i = 0; i < certSet.intermediates.size(); i++) {
        if (certSet.intermediates[i]) {
            std::cout << "    " << i << ": " << getCertId(certSet.intermediates[i].get()) << std::endl;
        } else {
            std::cout << "    " << i << ": NULL" << std::endl;
        }
    }

    // Load additional certificates (SM-DP+ auth, TLS, etc.)
    std::cout << "\nLoading superfluous certificates:" << std::endl;

    // Create filter function for consistency
    std::function<bool(const std::string&)> filterFunc = nullptr;
    if (!filter.empty()) {
        filterFunc = [&filter](const std::string& filename) {
            return filename.find(filter) != std::string::npos;
        };
    }

    auto dpAuthCerts = loadCertsFromDirectory(
        baseDir + "/DPauth",
        "SM-DP+ Auth certificates",
        "CERT_",
        filterFunc
    );

    auto dpPbCerts = loadCertsFromDirectory(
        baseDir + "/DPpb",
        "SM-DP+ PB certificates",
        "CERT_",
        filterFunc
    );

    // Build untrusted chain with ALL certificates (including superfluous ones)
    std::vector<X509*> messyChain;

    // Add the real intermediates
    std::cout << "\nBuilding messy untrusted chain:" << std::endl;
    std::cout << "Number of intermediates in certSet: " << certSet.intermediates.size() << std::endl;
    for (size_t i = 0; i < certSet.intermediates.size(); i++) {
        const auto& intermediate = certSet.intermediates[i];
        if (intermediate && intermediate.get()) {
            X509_NAME* name = X509_get_subject_name(intermediate.get());
            if (name) {
                std::cout << "  Adding required intermediate " << i << ": " <<
                            getCertId(intermediate.get()) << std::endl;
            } else {
                std::cout << "  WARNING: Intermediate " << i << " has NULL subject name!" << std::endl;
            }
            messyChain.push_back(intermediate.get());
        } else {
            std::cout << "  WARNING: Null intermediate certificate at index " << i << "!" << std::endl;
        }
    }

    // Add SM-DP+ auth certificates
    for (const auto& cert : dpAuthCerts) {
        if (cert) {
            std::cout << "  Adding SM-DP+ Auth: " << getCertId(cert.get()) << std::endl;
            messyChain.push_back(cert.get());
        }
    }

    // Add SM-DP+ PB certificates
    for (const auto& cert : dpPbCerts) {
        if (cert) {
            std::cout << "  Adding SM-DP+ PB: " << getCertId(cert.get()) << std::endl;
            messyChain.push_back(cert.get());
        }
    }

    std::cout << "\nTotal untrusted chain size: " << messyChain.size() << " certificates" << std::endl;
    std::cout << "(Including " << (messyChain.size() - certSet.intermediates.size()) << " superfluous certificates)" << std::endl;

    // Find the root that worked in the basic test
    int workingRootIndex = -1;
    for (size_t i = 0; i < certSet.roots.size(); i++) {
        // Quick test to find which root works
        std::vector<X509*> simpleChain;
        for (const auto& intermediate : certSet.intermediates) {
            simpleChain.push_back(intermediate.get());
        }

        try {
            bool result = CertificateUtil::verifyCertificateChainDynamic(
                certSet.euicc.get(), simpleChain, certSet.roots[i].get(), false);
            if (result) {
                workingRootIndex = i;
                break;
            }
        } catch (...) {}
    }

    if (workingRootIndex < 0) {
        std::cerr << "ERROR: Could not find working root certificate!" << std::endl;
        return;
    }

    // Test with the working root
    std::cout << "\nVerifying with messy chain..." << std::endl;
    std::cout << "Using root that worked in basic test: " <<
                 getCertId(certSet.roots[workingRootIndex].get()) << std::endl;

    try {
        bool result = CertificateUtil::verifyCertificateChainDynamic(
            certSet.euicc.get(),
            messyChain,
            certSet.roots[workingRootIndex].get(),
            true  // Verbose to see the discovered chain
        );

        if (result) {
            std::cout << "\nChain verification SUCCESSFUL even with superfluous certificates." << std::endl;
            std::cout << "OpenSSL correctly identified the valid chain from the messy pool." << std::endl;
        } else {
            std::cout << "\nChain verification FAILED with superfluous certificates" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "\nChain verification ERROR: " << e.what() << std::endl;
    }
}


int main(int argc, char* argv[]) {
    std::string certDir = "./sgp26";
    std::string filter = "NIST";  // Default to NIST certificates
    bool verbose = false;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-d" || arg == "--dir") {
            if (i + 1 < argc) {
                certDir = argv[++i];
            } else {
                std::cerr << "Error: -d/--dir requires a directory path" << std::endl;
                return 1;
            }
        } else if (arg == "-f" || arg == "--filter") {
            if (i + 1 < argc) {
                filter = argv[++i];
            } else {
                std::cerr << "Error: -f/--filter requires a filter string" << std::endl;
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -d, --dir <path>     Certificate directory (default: ./sgp26)" << std::endl;
            std::cout << "  -f, --filter <str>   Certificate filter (default: NIST, use BRP for BRP certs)" << std::endl;
            std::cout << "  -v, --verbose        Enable verbose output" << std::endl;
            std::cout << "  -h, --help           Show this help" << std::endl;
            return 0;
        }
    }

    std::cout << "Certificate Chain Verification Test Tool" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Certificate directory: " << certDir << std::endl;
    std::cout << "Certificate filter: " << filter << std::endl;
    std::cout << "Verbose mode: " << (verbose ? "enabled" : "disabled") << std::endl;

    try {
        // Initialize OpenSSL
        SSL_library_init();
        SSL_load_error_strings();
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();

        // Load and categorize certificates
        CertificateSet certSet = loadAndCategorizeCerts(certDir, filter);

        // Summary
        std::cout << "\n--- Certificate Summary ---" << std::endl;
        std::cout << "Root certificates loaded: " << certSet.roots.size() << std::endl;
        std::cout << "Intermediate certificates loaded: " << certSet.intermediates.size() << std::endl;
        std::cout << "eUICC certificate loaded: " << (certSet.euicc ? "YES" : "NO") << std::endl;

        // DEBUG: Print intermediate details
        std::cout << "\nDEBUG: Intermediate certificate details right after loading:" << std::endl;
        for (size_t i = 0; i < certSet.intermediates.size(); i++) {
            if (certSet.intermediates[i]) {
                std::cout << "  Intermediate " << i << ": " << getCertId(certSet.intermediates[i].get()) << std::endl;
            } else {
                std::cout << "  Intermediate " << i << ": NULL" << std::endl;
            }
        }

        if (certSet.roots.empty() || !certSet.euicc) {
            std::cerr << "\nERROR: Insufficient certificates loaded for testing!" << std::endl;
            return 1;
        }

        // Run verification tests
        std::cout << "\nDEBUG: Before testChainVerification - Intermediate 0: " <<
                  (certSet.intermediates.empty() ? "EMPTY" : getCertId(certSet.intermediates[0].get())) << std::endl;

        bool success = testChainVerification(certSet, verbose);

        std::cout << "\nDEBUG: After testChainVerification - Intermediate 0: " <<
                  (certSet.intermediates.empty() ? "EMPTY" : getCertId(certSet.intermediates[0].get())) << std::endl;

        // Test with superfluous certificates
        testWithSuperfluousCerts(certSet, certDir, filter);

        // Final result
        std::cout << "\n========================================" << std::endl;
        std::cout << "FINAL RESULT: " << (success ? "SUCCESS" : "FAILURE") << std::endl;
        std::cout << "========================================" << std::endl;

        // Cleanup OpenSSL
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
        ERR_free_strings();

        return success ? 0 : 1;

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 2;
    }
}