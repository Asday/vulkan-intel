-include config

rwildcard = \
	$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
sources = $(call rwildcard,src/,*.cpp)
cmds = $(wildcard src/cmd/*.cpp)
base = build

.PHONY = all clean compile print-sources print-cmds

.SECONDARY:


define template =
.PHONY += all-$1
all-$1: CPPFLAGS = $(BASE_CPPFLAGS) $($1-CPPFLAGS)
all-$1: $(foreach
	cmd,\
	$(cmds),\
	$(base)/$1/bin/$(basename $(cmd:src/cmd/%=%))\
)

.PHONY += compile-$1
compile-$1: $(foreach
	source,\
	$(sources),\
	$(base)/$1/o/$(basename $(source:src/%=%)).o\
)
$(base)/$1/o/%.o: CPPFLAGS = $(BASE_CPPFLAGS) $($1-CPPFLAGS)
$(base)/$1/o/%.o: src/%.cpp config
	mkdir -p "$$(@D)"
	mkdir -p "$(base)/$1/d/$$(*D)"

	$$(CXX) -c $$(CPPFLAGS) -MMD -MP -MF "$(base)/$1/d/$$*.d" $$< -o $$@

-include $(foreach
	source,\
	$(sources),\
	$(base)/$1/d/$(basename $(source:src/%=%)).d\
)

$(base)/$1/bin/%: $(base)/$1/o/cmd/%.o compile-$1
	mkdir -p "$$(@D)"
	$$(CXX) \
		$$(LDFLAGS) \
		$$< $$(filter-out \
			$(base)/$1/o/cmd/%,\
			$$(call rwildcard,$(base)/$1/o/,*.o)\
		) \
		-o $$@
endef


all: $(foreach config,$(configs),all-$(config))
compile: $(foreach config,$(configs),compile-$(config))

$(foreach config,$(configs),$(eval $(call template,$(config))))

%: $(base)/$(word 1,$(configs))/bin/%
	./$<

clean:
	rm -rf $(base)/

# "Why does the makefile not work?" helpers
print-sources:
	echo "$(sources)"

print-cmds:
	echo "$(cmds)"
