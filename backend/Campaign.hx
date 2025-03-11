package backend;

import backend.Extension;

// TODO: Find better name
@:nullSafety(Strict)
typedef Campaign = {
  floorSize: UInt,
  extensions: Array<Extension>
};
