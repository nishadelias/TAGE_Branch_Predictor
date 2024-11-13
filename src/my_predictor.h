// // my_predictor.h

#include <vector>
#include <cmath>

// 30, 19, 23, 17 = 5.013

#define HISTORY_LENGTH 30
#define BASE_TABLE_BITS 19
#define TAG_TABLE_BITS 23
#define TAG_BITS 17
#define NUM_COMPONENTS 4

class my_update : public branch_update {
public:
    std::vector<unsigned int> indices;
    std::vector<unsigned int> tags;
    std::vector<bool> predictions;
    unsigned int base_prediction;
    int chosen_component;
    bool base_prediction_used;
};

struct tagged_entry {
    unsigned char counter;  // 3-bit prediction counter
    unsigned char useful;   // 2-bit useful counter
    unsigned int tag;      // Tag bits
};

class my_predictor : public branch_predictor {
public:
    my_update u;
    branch_info bi;
    unsigned int global_history;
    std::vector<std::vector<tagged_entry> > tagged_tables;
    std::vector<unsigned int> history_lengths;
    unsigned char base_predictor[1 << BASE_TABLE_BITS];

    my_predictor(void) : global_history(0) {
        // Initialize base predictor
        memset(base_predictor, 2, sizeof(base_predictor));  // Initialize to weakly taken

        // Initialize history lengths geometrically
        history_lengths.push_back(6);
        history_lengths.push_back(12);     
        history_lengths.push_back(19);
        history_lengths.push_back(30);
        

        // Initialize tagged tables
        for (unsigned int i = 0; i < NUM_COMPONENTS; i++) {
            tagged_tables.push_back(std::vector<tagged_entry>(1 << TAG_TABLE_BITS));
            for (auto& entry : tagged_tables[i]) {
                entry.counter = 2;  // Initialize to weakly taken
                entry.useful = 0;
                entry.tag = 0;
            }
        }
    }

    // Compute index for a tagged component
    unsigned int compute_index(unsigned int pc, unsigned int history, unsigned int comp_id) {
        unsigned int index = pc ^ (pc >> (abs((int)history_lengths[comp_id] - (int)comp_id) + 1));
        index ^= history & ((1 << history_lengths[comp_id]) - 1);
        return index & ((1 << TAG_TABLE_BITS) - 1);
    }

    // Compute tag for a tagged component
    unsigned int compute_tag(unsigned int pc, unsigned int history, unsigned int comp_id) {
        unsigned int tag = pc ^ (history & ((1 << history_lengths[comp_id]) - 1));
        tag ^= (history >> history_lengths[comp_id]) & ((1 << TAG_BITS) - 1);
        return tag & ((1 << TAG_BITS) - 1);
    }

    // Get prediction from counter value
    bool get_prediction(unsigned char counter) {
        return counter >= 4;  // Predict taken if counter >= 4
    }

    branch_update* predict(branch_info& b) {
        bi = b;
        if (!(b.br_flags & BR_CONDITIONAL)) {
            u.direction_prediction(true);
            return &u;
        }

        u.indices.clear();
        u.tags.clear();
        u.predictions.clear();
        
        // Compute base prediction
        unsigned int base_index = b.address & ((1 << BASE_TABLE_BITS) - 1);
        u.base_prediction = get_prediction(base_predictor[base_index]);

        // Initialize as using base predictor
        u.chosen_component = -1;
        u.base_prediction_used = true;

        // Check all tagged predictors
        for (unsigned int i = 0; i < NUM_COMPONENTS; i++) {
            unsigned int index = compute_index(b.address, global_history, i);
            unsigned int tag = compute_tag(b.address, global_history, i);
            
            u.indices.push_back(index);
            u.tags.push_back(tag);
            
            tagged_entry& entry = tagged_tables[i][index];
            bool prediction = get_prediction(entry.counter);
            u.predictions.push_back(prediction);

            if (entry.tag == tag) {  // Tag match found
                u.chosen_component = i;
                u.base_prediction_used = false;
                u.direction_prediction(prediction);
                break;
            }
        }

        if (u.base_prediction_used) {
            u.direction_prediction(u.base_prediction);
        }

        return &u;
    }

    void update(branch_update* u, bool taken, unsigned int target) {
        if (!(bi.br_flags & BR_CONDITIONAL)) return;

        my_update* mu = static_cast<my_update*>(u);
        
        // Update base predictor if used
        if (mu->base_prediction_used) {
            unsigned int base_index = bi.address & ((1 << BASE_TABLE_BITS) - 1);
            if (taken && base_predictor[base_index] < 7) base_predictor[base_index]++;
            if (!taken && base_predictor[base_index] > 0) base_predictor[base_index]--;
        }

        // Update tagged components
        if (mu->chosen_component >= 0) {
            // Update prediction counter for chosen component
            tagged_entry& chosen = tagged_tables[mu->chosen_component][mu->indices[mu->chosen_component]];
            if (taken && chosen.counter < 7) chosen.counter++;
            if (!taken && chosen.counter > 0) chosen.counter--;

            // Update useful counter
            if (mu->predictions[mu->chosen_component] != mu->base_prediction) {
                if (mu->predictions[mu->chosen_component] == taken) {
                    if (chosen.useful < 3) chosen.useful++;
                } else {
                    if (chosen.useful > 0) chosen.useful--;
                }
            }
        }

        // Allocate new entries when prediction is incorrect
        if ((mu->base_prediction_used && mu->base_prediction != taken) ||
            (!mu->base_prediction_used && mu->predictions[mu->chosen_component] != taken)) {
            
            // Try to allocate in components after the chosen one
            int start = (mu->chosen_component < 0) ? 0 : mu->chosen_component + 1;
            for (int i = start; i < NUM_COMPONENTS; i++) {
                tagged_entry& entry = tagged_tables[i][mu->indices[i]];
                if (entry.useful == 0) {
                    entry.tag = mu->tags[i];
                    entry.counter = taken ? 4 : 3;  // Initialize weakly
                    entry.useful = 0;
                    break;
                }
            }
        }

        // Update global history
        global_history <<= 1;
        global_history |= taken;
        global_history &= (1 << HISTORY_LENGTH) - 1;
    }
};