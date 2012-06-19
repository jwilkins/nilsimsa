SPEC = Gem::Specification.new do |spec|
  # Descriptive and source information for this gem.
  spec.name = "nilsimsa"
  spec.version = "1.1.0"
  spec.summary = "Computes Nilsimsa values.  Nilsimsa is a distance based hash"
  spec.author = "Jonathan Wilkins"
  spec.email = "jwilkins[at]nospam[dot]bitland[dot]net"
  spec.has_rdoc = true
  spec.extra_rdoc_files = ["README"]

  spec.files = %w(Gemfile Rakefile README nilsimsa.gemspec
                  lib/nilsimsa.rb
                  bin/nilsimsa
                  examples/simple.rb
                  ext/extconf.rb ext/nilsimsa.c)
  spec.executables = ['nilsimsa']

  # optional native component
  spec.extensions = ['ext/extconf.rb']
end
