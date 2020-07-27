ifndef MESSAGELEN
    $(error MESSAGELEN is not set)
endif

ifndef STEGOLEN
    $(error STEGOLEN is not set)
endif

ifndef TRELLISH
    $(error TRELLISH is not set)
endif

SRCDIR := src
OBJDIR := obj
DEPDIR := .deps

TARGET := sampler

SRCS := $(wildcard $(SRCDIR)/*.cxx)
OBJS := $(patsubst $(SRCDIR)/%.cxx,$(OBJDIR)/%.o,$(SRCS))
DEPS := $(patsubst $(SRCDIR)/%.cxx,$(DEPDIR)/%.d,$(SRCS))

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
CPPFLAGS += -pthread -std=c++11 -O3 -D MODELNAME=$(MODELNAME) -D TRELLISH=$(TRELLISH) -D MESSAGELEN=$(MESSAGELEN) -D STEGOLEN=$(STEGOLEN)

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
