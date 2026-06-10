CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99

.PHONY: test clean

test: phase1_helpers_test sh4_emit_encoding_test sh4_integration_contract_test \
	phase2_ldm_stm_test phase3_memory_test phase4_cheats_test \
	phase5_user_readiness_test phase6_release_verification_test \
	dc_build_contract_test phase8_dreamcast_polish_test \
	phase9_stability_test phase10_external_validation_test
	./phase1_helpers_test
	./sh4_emit_encoding_test
	./sh4_integration_contract_test
	./phase2_ldm_stm_test
	./phase3_memory_test
	./phase4_cheats_test
	./phase5_user_readiness_test
	./phase6_release_verification_test
	./dc_build_contract_test
	./phase8_dreamcast_polish_test
	./phase9_stability_test
	./phase10_external_validation_test

phase1_helpers_test: phase1_helpers_test.c
	$(CC) $(CFLAGS) -o $@ $<

sh4_emit_encoding_test: sh4_emit_encoding_test.c
	$(CC) $(CFLAGS) -I.. -o $@ $<

sh4_integration_contract_test: sh4_integration_contract_test.c
	$(CC) $(CFLAGS) -o $@ $<

phase2_ldm_stm_test: phase2_ldm_stm_test.c
	$(CC) $(CFLAGS) -o $@ $<

phase3_memory_test: phase3_memory_test.c
	$(CC) $(CFLAGS) -o $@ $<

phase4_cheats_test: phase4_cheats_test.c
	$(CC) $(CFLAGS) -o $@ $<

phase5_user_readiness_test: phase5_user_readiness_test.c
	$(CC) $(CFLAGS) -o $@ $<

phase6_release_verification_test: phase6_release_verification_test.c
	$(CC) $(CFLAGS) -o $@ $<

dc_build_contract_test: dc_build_contract_test.c
	$(CC) $(CFLAGS) -o $@ $<

phase8_dreamcast_polish_test: phase8_dreamcast_polish_test.c
	$(CC) $(CFLAGS) -o $@ $<

phase9_stability_test: phase9_stability_test.c
	$(CC) $(CFLAGS) -o $@ $<

phase10_external_validation_test: phase10_external_validation_test.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f phase1_helpers_test sh4_emit_encoding_test \
		sh4_integration_contract_test phase2_ldm_stm_test phase3_memory_test \
		phase4_cheats_test phase5_user_readiness_test \
		phase6_release_verification_test dc_build_contract_test \
		phase8_dreamcast_polish_test phase9_stability_test \
		phase10_external_validation_test
