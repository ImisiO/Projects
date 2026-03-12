using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.Models
{
    public class DialogueConnection
    {
        #region Member Variables

        /// <summary>Connection Id</summary>
        public Guid Id { get; init; } = Guid.NewGuid();
        
        /// <summary>The id of the node the connection is starting from</summary>
        public Guid FromId { get; set; }

        /// <summary>The id of the node that the connection is going to</summary>
        public Guid ToId { get; set; }

        #endregion  // Member Variables
    }
}
