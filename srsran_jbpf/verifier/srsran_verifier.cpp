
#include "srsran_verifier.hpp"
#include "jbpf_helper_api_defs_ext.h"
#include "context_descriptors.hpp"
#include "jbpf_srsran_contexts.h"



using std::string;
using std::vector;



jbpf_verifier_result_t
srsran_verify(const char* objfile, const char* section, const char* asmfile)
{
    // register jbpf_srsran_ofh program type
    {
        // Next, we define the new program type
        EbpfProgramType new_program_type;
        new_program_type.name = "jbpf_srsran_ofh";
        new_program_type.context_descriptor = &jbpf_srsran_ofh_descr;
        new_program_type.section_prefixes = {
            "jbpf_srsran_ofh"}; // The name(s) used to define the ELF section,
                               // where the codelet function is stored for this program type
        new_program_type.is_privileged = false;
        jbpf_verifier_register_program_type(JBPF_PROG_TYPE_SRSRAN_OFH, new_program_type);
    }

    // register jbpf_srsran_layer2 program type
    {
        // Next, we define the new program type
        EbpfProgramType new_program_type;
        new_program_type.name = "jbpf_srsran_layer2";
        new_program_type.context_descriptor = &jbpf_srsran_layer2_descr;
        new_program_type.section_prefixes = {
            "jbpf_srsran_layer2"}; // The name(s) used to define the ELF section,
                               // where the codelet function is stored for this program type
        new_program_type.is_privileged = false;
        jbpf_verifier_register_program_type(JBPF_PROG_TYPE_SRSRAN_OFH, new_program_type);
    }

    return jbpf_verify(objfile, section, asmfile);
}
