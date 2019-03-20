#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cassert>

#include "core/types.h"
#include "interface/protocols.h"
#include "system/system_call.h"
#include "core/Parameter.h"

#include "smc_weight.h"

double smc_weight(const cmd_t& perturbation_pdf,
                  const double prmtr_prior_pdf,
                  const int t,
                  const std::vector<Parameter>& prmtr_accepted_old,
                  const std::vector<double>& weights_old,
                  const Parameter& prmtr_perturbed)
{

    using namespace std;

    // Sanity check: prmtr_accepted_old and weights_old should have the same
    // size
    assert(prmtr_accepted_old.size() == weights_old.size());

    // If in generation 0, return uniform weight
    if (t == 0)
        return 1.0 / ((double) prmtr_accepted_old.size());

    // Prepare input to perturbation_pdf
    std::string perturbation_pdf_input =
        format_perturbation_pdf_input(t, prmtr_perturbed, prmtr_accepted_old);

    // Call perturbation_pdf
    std::string perturbation_pdf_output;
    system_call(perturbation_pdf, perturbation_pdf_input,
            perturbation_pdf_output);

    // Compute denominator
    std::vector<double> perturbation_pdf_old =
        parse_perturbation_pdf_output(perturbation_pdf_output);
    double denominator = 0.0;

    try
    {
        for (int i = 0; i < weights_old.size(); i++)
            denominator += weights_old[i] * perturbation_pdf_old.at(i);
    }
    catch (const std::out_of_range& e)
    {
        std::string error_msg;
        error_msg += "Out of range: ";
        error_msg += e.what();
        error_msg += '\n';
        error_msg += "Perturbation_pdf did not output enough parameters";
        throw std::runtime_error(error_msg);
    }

    // Return weight
    return prmtr_prior_pdf / denominator;
}