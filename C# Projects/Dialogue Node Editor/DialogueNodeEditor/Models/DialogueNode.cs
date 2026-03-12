namespace DialogueNodeEditor.Models
{
    public class DialogueNode
    {
        /// <summary>Id of dialogue node</summary>
        public Guid Id { get; init; } = Guid.NewGuid();
        
        /// <summary>Dialogue Id</summary>
        public string DialogueId { get; set; } = "New Dialogue ...";
        
        /// <summary>Owner of the dialogue string</summary>
        public string Owner { get; set; } = "Owner ...";

        /// <summary>Per-locale dialogue text, keyed by translation key (e.g. "en_us"). </summary>
        public Dictionary<string, string> DialogueTranslations { get; set; } = new();

        /// <summary>Optional notes / comments</summary>
        public string Notes { get; set; } = "";

        /// <summary>Node x position</summary>
        public double X { get; set; } = 100;
        
        /// <summary>Node y position</summary>
        public double Y { get; set; } = 100;
    }
}