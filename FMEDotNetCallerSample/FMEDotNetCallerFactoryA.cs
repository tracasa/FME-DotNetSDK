using System;
using System.Collections.Generic;
using System.Text;

namespace Safe.DotNet.Samples
{
    /// <summary> 
    /// Implements a basic sample of a DotNet custom Transformer. 
    /// </summary>
    public class FMEDotNetCallerFactoryA : Safe.DotNet.FMEDotNetFactory
    {
        /// <summary> Reference to the plugin SDK bridge. </summary>
        private IFMEOFactoryBridge _bridge;

        /// <summary> Initialize the object in the current task. </summary>
        public override void Initialize(IFMEOFactoryBridge bridge)
        {
            _bridge = bridge;
            _bridge.LogFile.LogMessageString("## "+GetType().Name+" - Initialize()", FMEOMessageLevel.Inform);
        }

        /// <summary> Process the specified Feature. </summary>
        public override IEnumerable<KeyValuePair<string,IFMEOFeature>> ProcessFeature(string inputTag, IFMEOFeature feature)
        {
            // Depending on how we have chosen to process features in this factory we could:
            // a) Process and output each feature immediately or
            // b) Process and store features into a list, then when all features have been collected, process and output them.
            //    Under a) the bulk of the work occurs in this function since features are output immediately as soon as they are done processing.
            //    Under b) the bulk of the work occurs in the close() function.
            if (feature!=null)
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("## "+GetType().Name+" - Processing Feature, Attributes=[");
                foreach (string attrib in feature.GetAllAttributeNames()) sb.Append(attrib).Append("='").Append(feature.GetAttributeAsString(attrib)).Append("', ");
                sb.Length-=2; sb.Append("]");
                _bridge.LogFile.LogMessageString(sb.ToString(), FMEOMessageLevel.Inform);

                // For this sample we output the feature with a specific OutputTag.
                // For this module the related fmx file defines one unique output tag called "OUTPUT"...
                yield return new KeyValuePair<string,IFMEOFeature>("OUTPUT", feature);
                // But we can use the '_bridge.OutputTags' member to query the available collection of output tags...
                // foreach (string outputTag in _bridge.OutputTags) yield return new KeyValuePair<string,IFMEOFeature>(outputTag, feature);                
            }
            yield break;
        }

        /// <summary> Abort the current task. </summary>
        public override void Abort()
        {
            _bridge.LogFile.LogMessageString("## "+GetType().Name+" - Abort()", FMEOMessageLevel.Warn);
        }

        /// <summary> Close the current task. </summary>
        public override void Close()
        {
            _bridge.LogFile.LogMessageString("## "+GetType().Name+" - Close()", FMEOMessageLevel.Inform);
        }

        /// <summary> Releases the resources of the object. </summary>
        public override void DisposeObject()
        {
            _bridge.LogFile.LogMessageString("## "+GetType().Name+" - Dispose()", FMEOMessageLevel.Inform);
            _bridge = null;
        }
    }
}
