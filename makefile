SRCDIR := src
OBJDIR := obj
DEPDIR := .deps

TARGET := sampler

SRCS := $(wildcard $(SRCDIR)/*.cxx)
OBJS := $(patsubst $(SRCDIR)/%.cxx,$(OBJDIR)/%.o,$(SRCS))
DEPS := $(patsubst $(SRCDIR)/%.cxx,$(DEPDIR)/%.d,$(SRCS))

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
CPPFLAGS += -std=c++11 -O3

sampler : $(OBJDIR) $(OBJS)
	$(CXX) $(CPPFLAGS) $(OBJS) -o $(TARGET)

clean :
	-rm -rf $(OBJDIR) $(DEPDIR) $(TARGET)

$(OBJDIR)/%.o : $(SRCDIR)/%.cxx $(OBJDIR) $(DEPDIR)/%.d | $(DEPDIR)
	$(CXX) $(DEPFLAGS) $(CPPFLAGS) -c $< -o $@

$(DEPDIR): ; @mkdir -p $@

$(OBJDIR): ; @mkdir -p $@

$(DEPS):

include $(wildcard $(DEPS))
