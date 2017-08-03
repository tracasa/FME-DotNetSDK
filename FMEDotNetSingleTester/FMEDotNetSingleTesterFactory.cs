using System;
using System.Collections.Generic;
using System.Text;

namespace Safe.DotNet
{
    /// <summary> 
    /// Implements a tester only allows a single test clause but it runs faster than generic tester.
    /// </summary>
    public class FMEDotNetSingleTesterFactory : Safe.DotNet.FMEDotNetFactory
    {
        /// <summary> Reference to the plugin SDK bridge. </summary>
        private IFMEOFactoryBridge _bridge;

    	/// <summary> English CultureInfo of the framework. </summary>
	    static System.Globalization.CultureInfo s_englishCultureInfo = new System.Globalization.CultureInfo("en-us");

        /// <summary> Defines the available operators of a test clause. </summary>
        private enum OperatorEnum : int
        {
            Equal          = 0,
            NotEqual       = 1,
            Less           = 2,
            LessOrEqual    = 3,
            Greater        = 4,
            GreaterOrEqual = 5,
            BeginsWith     = 6,
            EndsWith       = 7,
            HasAvalidValue = 8,
            IsMissing      = 9,            
        }
        /// <summary> Current operator to test. </summary>
        private OperatorEnum _operatorEnum = OperatorEnum.Equal;

        /// <summary> Left hand operand. </summary>
        private string _lhandKeyword = string.Empty;
        /// <summary> Right hand operand. </summary>
        private string _rhandKeyword = string.Empty;
        /// <summary> Predefined values to test. </summary>
        private Dictionary<string,int> _rvalueHash = new Dictionary<string,int>();
        /// <summary> Predefined numbers to test. </summary>
        private List<double> _rvalueList = new List<double>();

        /// <summary> Fix the quoted empty string. </summary>
        private static string FixQuotedEmptyString(string value)
        {
            if (value!=null && value.Length==2)
            {
                char f = value[0];
                char l = value[1];
                if ((f=='\'' && l=='\'') || (f=='\"' && l=='\"')) return string.Empty;
            }
            return value;
        }
        /// <summary> Returns the value of the specified attribute or expression as String. </summary>
        private static string GetOrResolveAttributeValue(IFMEOFeature feature, string value)
        {
            if (value!=null && value.Length>0)
            {
                if (value.StartsWith("@"))
                {
                    value = feature.GetOrResolveAttributeAsString(value);
                    if (value!=null && value.StartsWith("@")) value = null;
                }
                else
                {
                    value = feature.GetAttributeAsString(value);
                }
            }
            return value;
        }

        /// <summary> Initialize the object in the current task. </summary>
        public override void Initialize(IFMEOFactoryBridge bridge)
        {
            string keyword = string.Empty;

            // Parse keywords.
            if (bridge.Keywords.TryGetValue("LHS_ATTR", out keyword))
            {
                _lhandKeyword = keyword;
            }
            if (bridge.Keywords.TryGetValue("OPERATOR_ATTR", out keyword))
            {
                if (keyword=="<" ) _operatorEnum = OperatorEnum.Less; else
                if (keyword=="<=") _operatorEnum = OperatorEnum.LessOrEqual; else
                if (keyword==">" ) _operatorEnum = OperatorEnum.Greater; else
                if (keyword==">=") _operatorEnum = OperatorEnum.GreaterOrEqual; else
                if (keyword=="!=") _operatorEnum = OperatorEnum.NotEqual; else
                if (keyword.Equals("containsvalue", StringComparison.CurrentCultureIgnoreCase)) _operatorEnum = OperatorEnum.HasAvalidValue; else
                if (keyword.Equals("nonemptyvalue", StringComparison.CurrentCultureIgnoreCase)) _operatorEnum = OperatorEnum.HasAvalidValue; else
                if (keyword.StartsWith("begins", StringComparison.CurrentCultureIgnoreCase)) _operatorEnum = OperatorEnum.BeginsWith; else
                if (keyword.StartsWith("ends", StringComparison.CurrentCultureIgnoreCase)) _operatorEnum = OperatorEnum.EndsWith; else
                if (keyword.EndsWith("missing", StringComparison.CurrentCultureIgnoreCase)) _operatorEnum = OperatorEnum.IsMissing; else
                _operatorEnum = OperatorEnum.Equal;
            }
            if (bridge.Keywords.TryGetValue("RHS_ATTR", out keyword) && !(_rhandKeyword = keyword).StartsWith("@"))
            {
                foreach (string tempText in _rhandKeyword.Split(new char[2]{',',';'}, StringSplitOptions.RemoveEmptyEntries))
                {
                    if (_operatorEnum>=OperatorEnum.Less && _operatorEnum<=OperatorEnum.GreaterOrEqual)
                    {
                        _rvalueList.Add(Double.Parse(tempText, s_englishCultureInfo));
                    }
                    _rvalueHash[tempText] = 1;
                }
            }
            _bridge = bridge;
        }

        /// <summary> Process the specified Feature. </summary>
        public override IEnumerable<KeyValuePair<string,IFMEOFeature>> ProcessFeature(string inputTag, IFMEOFeature feature)
        {
            if (feature!=null)
            {
                string lvalue = GetOrResolveAttributeValue(feature, _lhandKeyword);
                string rvalue = _rhandKeyword;
                if (_rvalueHash.Count==0) rvalue = GetOrResolveAttributeValue(feature, _rhandKeyword);
                bool ok = false;

                // First checks and validations.
                if (lvalue==null)
                {
                    ok = _operatorEnum==OperatorEnum.IsMissing;
                    goto gotoReturnFeature;
                }
                lvalue = FixQuotedEmptyString(lvalue);
                rvalue = FixQuotedEmptyString(rvalue);

                // Compare the attribute value.
                switch (_operatorEnum)
                {
                    case OperatorEnum.HasAvalidValue:
                        ok = lvalue!=null && lvalue.Length>0;
                        break;
                    case OperatorEnum.IsMissing:
                        ok = lvalue==null;
                        break;
                    case OperatorEnum.NotEqual:
                        ok = _rvalueHash.Count>0 ? !_rvalueHash.ContainsKey(lvalue) : !rvalue.Equals(lvalue);
                        break;

                    case OperatorEnum.BeginsWith:
                    {
                        if (_rvalueHash.Count>0)
                        {
                            foreach (string rv in _rvalueHash.Keys) if (lvalue.StartsWith(rv)) { ok = true; break; }
                        }
                        else
                        {
                            ok = lvalue.StartsWith(rvalue);
                        }
                        break;
                    }
                    case OperatorEnum.EndsWith:
                    {
                        if (_rvalueHash.Count>0)
                        {
                            foreach (string rv in _rvalueHash.Keys) if (lvalue.EndsWith(rv)) { ok = true; break; }
                        }
                        else
                        {
                            ok = lvalue.EndsWith(rvalue);
                        }
                        break;
                    }
                    case OperatorEnum.Less:
                    {
                        double doubleValue = double.Parse(lvalue, s_englishCultureInfo);

                        if (_rvalueList.Count>0)
                        {
                            foreach (double rv in _rvalueList) if (doubleValue < rv) { ok = true; break; }
                        }
                        else
                        {
                            double rv = double.Parse(rvalue, s_englishCultureInfo);
                            ok = doubleValue < rv;
                        }
                        break;
                    }
                    case OperatorEnum.LessOrEqual:
                    {
                        double doubleValue = double.Parse(lvalue, s_englishCultureInfo);

                        if (_rvalueList.Count>0)
                        {
                            foreach (double rv in _rvalueList) if (doubleValue <= rv) { ok = true; break; }
                        }
                        else
                        {
                            double rv = double.Parse(rvalue, s_englishCultureInfo);
                            ok = doubleValue <= rv;
                        }
                        break;
                    }
                    case OperatorEnum.Greater:
                    {
                        double doubleValue = double.Parse(lvalue, s_englishCultureInfo);

                        if (_rvalueList.Count>0)
                        {
                            foreach (double rv in _rvalueList) if (doubleValue > rv) { ok = true; break; }
                        }
                        else
                        {
                            double rv = double.Parse(rvalue, s_englishCultureInfo);
                            ok = doubleValue > rv;
                        }
                        break;
                    }
                    case OperatorEnum.GreaterOrEqual:
                    {
                        double doubleValue = double.Parse(lvalue, s_englishCultureInfo);

                        if (_rvalueList.Count>0)
                        {
                            foreach (double rv in _rvalueList) if (doubleValue >= rv) { ok = true; break; }
                        }
                        else
                        {
                            double rv = double.Parse(rvalue, s_englishCultureInfo);
                            ok = doubleValue >= rv;
                        }
                        break;
                    }
                    default:
                        ok = _rvalueHash.Count>0 ? _rvalueHash.ContainsKey(lvalue) : rvalue.Equals(lvalue);
                        break;
                }
            gotoReturnFeature:
                yield return new KeyValuePair<string,IFMEOFeature>(ok ? "PASSED" : "FAILED", feature);
            }
            yield break;
        }

        /// <summary> Abort the current task. </summary>
        public override void Abort()
        {
        }

        /// <summary> Close the current task. </summary>
        public override void Close()
        {
        }

        /// <summary> Releases the resources of the object. </summary>
        public override void DisposeObject()
        {
            _bridge = null;
        }
    }
}
