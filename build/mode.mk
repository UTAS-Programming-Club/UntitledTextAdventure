ifndef MAKECMDGOALS
MODE := debug
else # MAKECMDGOALS
ifneq (,$(call findany,$(DEBUG_TARGETS),$(MAKECMDGOALS)))
MODE := debug
else # !debug targets
ifneq (,$(call findany,$(RELEASE_TARGETS),$(MAKECMDGOALS)))
MODE := release
endif # release targets
endif # debug/!debug targets
endif # MAKECMDGOALS
