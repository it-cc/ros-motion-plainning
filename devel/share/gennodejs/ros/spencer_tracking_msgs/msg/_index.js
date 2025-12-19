
"use strict";

let DetectedPersons = require('./DetectedPersons.js');
let TrackedPerson = require('./TrackedPerson.js');
let PersonTrajectory = require('./PersonTrajectory.js');
let TrackedPerson2d = require('./TrackedPerson2d.js');
let TrackedPersons2d = require('./TrackedPersons2d.js');
let TrackedPersons = require('./TrackedPersons.js');
let CompositeDetectedPersons = require('./CompositeDetectedPersons.js');
let DetectedPerson = require('./DetectedPerson.js');
let TrackingTimingMetrics = require('./TrackingTimingMetrics.js');
let TrackedGroup = require('./TrackedGroup.js');
let TrackedGroups = require('./TrackedGroups.js');
let CompositeDetectedPerson = require('./CompositeDetectedPerson.js');
let ImmDebugInfos = require('./ImmDebugInfos.js');
let ImmDebugInfo = require('./ImmDebugInfo.js');
let PersonTrajectoryEntry = require('./PersonTrajectoryEntry.js');

module.exports = {
  DetectedPersons: DetectedPersons,
  TrackedPerson: TrackedPerson,
  PersonTrajectory: PersonTrajectory,
  TrackedPerson2d: TrackedPerson2d,
  TrackedPersons2d: TrackedPersons2d,
  TrackedPersons: TrackedPersons,
  CompositeDetectedPersons: CompositeDetectedPersons,
  DetectedPerson: DetectedPerson,
  TrackingTimingMetrics: TrackingTimingMetrics,
  TrackedGroup: TrackedGroup,
  TrackedGroups: TrackedGroups,
  CompositeDetectedPerson: CompositeDetectedPerson,
  ImmDebugInfos: ImmDebugInfos,
  ImmDebugInfo: ImmDebugInfo,
  PersonTrajectoryEntry: PersonTrajectoryEntry,
};
