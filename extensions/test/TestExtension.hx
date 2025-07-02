package extensions.test;

import backend.Extension;
import extensions.test.Rooms;

@:nullSafety(Strict)
final TestExt: Extension = {
  module: 'extensions.test',
  actions: [],
  outcomes: [],
  screens: [
    TestRoom,
  ],
};
